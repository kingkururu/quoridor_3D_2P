#include "game.hpp" 

// GameManager constructor sets up the window, initializes constant variables, calls the random function, and makes scenes 
GameManager::GameManager()
    : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT) {
    introScene = std::make_unique<lobbyScene>(mainWindow.getWindow());
    introScene2 = std::make_unique<lobby2Scene>(mainWindow.getWindow());
    gameScene = std::make_unique<gamePlayScene>(mainWindow.getWindow());

    #if RUN_NETWORK
    // Initialize network state
    isNetworkEnabled = false;
    networkRole = NetworkRole::NONE;
    lastSyncTime = 0.0f;
    syncInterval = 1.0f / Constants::FRAME_LIMIT; // Sync at 60 FPS max
    #endif

    log_info("\tGame initialized");
}

GameManager::~GameManager() {
    log_info("GameManager destructor called");
    
    #if RUN_NETWORK
    if (isNetworkEnabled) {
        isNetworkEnabled = false;
        net.cleanup();
    }
    #endif
    
    // Force application exit if it's still hanging
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    log_info("GameManager destructor finished");
}

void GameManager::runGame() {
    try {     
        loadScenes(); 

        while (mainWindow.getWindow().isOpen()) {
            countTime();
            
            resetFlags();

            #if RUN_NETWORK                
            if (isNetworkEnabled) {
                handleNetworkMessages();
                // Only sync at specified intervals to reduce network spam
                if (net.isNetworkConnected()) {
                    if (MetaComponents::globalTime - lastSyncTime >= syncInterval) {
                        syncGameState();
                        lastSyncTime = MetaComponents::globalTime;
                    }
                }
            } 
            #endif
            
            handleEventInput();

            runScenesFlags(); 
        }
        log_info("\tGame Ended\n"); 
                    
    } catch (const std::exception& e) {
        log_error("Exception in runGame: " + std::string(e.what())); 
        mainWindow.getWindow().close(); 
    }
}

void GameManager::runScenesFlags() {
    bool currentlyInLobby1 = FlagSystem::lobbyEvents.sceneStart && !FlagSystem::lobbyEvents.sceneEnd;
    bool currentlyInLobby2 = FlagSystem::lobby2Events.sceneStart && !FlagSystem::lobby2Events.sceneEnd;
    bool currentlyInGame = FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd;

    // Run the active scene
    if (currentlyInLobby1) introScene->runScene();
    if (currentlyInLobby2){
        introScene2->runScene(); // runScene has to go first

        MetaComponents::hostIP = net.getLocalIP();
        if(FlagSystem::lobby2Events.hostButtonClicked) startHosting();
        else if (MetaComponents::inputText != "") startClient();
    }

    else if (currentlyInGame) {
        // Initialize turn states when game starts
        if (!gameSceneTurnInitialized) {
            #if RUN_NETWORK
            if (isNetworkEnabled) {
                MetaComponents::hostTurn = FlagSystem::gameScene1Flags.playerBlueTurn;
                MetaComponents::clientTurn = FlagSystem::gameScene1Flags.playerRedTurn;
            }         
            #endif
            gameSceneTurnInitialized = true;
        }
        gameScene->runScene();
    }
}

void GameManager::loadScenes(){
    introScene->createAssets(); 
    gameScene->createAssets();
    introScene2->createAssets();
}

// countTime counts global time and delta time for scenes to later use in runScene 
void GameManager::countTime() {
    sf::Time frameTime = MetaComponents::clock.restart();
    MetaComponents::deltaTime = frameTime.asSeconds(); 
    MetaComponents::globalTime += MetaComponents::deltaTime;
}

void GameManager::handleEventInput() {
    sf::Event event;
    while (mainWindow.getWindow().pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            log_info("Window close event detected.");
            FlagSystem::flagEvents.gameEnd = true;
            mainWindow.getWindow().close();

            #if RUN_NETWORK
            isNetworkEnabled = false;
            #endif

            return; 
        }

        if (isHost() && !net.isNetworkConnected()) return; // no inputs if client hasn't joined yet
        
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
            bool isPressed = (event.type == sf::Event::KeyPressed); 
            switch (event.key.code) {
                case sf::Keyboard::A: 
                    #if RUN_NETWORK
                    // Host: Apply input locally
                    if (isHost() || !isNetworkEnabled) FlagSystem::flagEvents.aPressed = isPressed; 
                    // Client: Send input state over network
                    if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("INPUT_STATE", "A:" + std::string(isPressed ? "1" : "0"));
                    #else
                    FlagSystem::flagEvents.aPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::W: 
                case sf::Keyboard::S: 
                {
                    bool isPressed = (event.type == sf::Event::KeyPressed);
                    #if RUN_NETWORK
                    if (isNetworkEnabled) {
                        // Only process if it's our turn and key is pressed
                        if (isPressed && ((isHost() && MetaComponents::hostTurn) || (!isHost() && MetaComponents::clientTurn))) {
                            // Set the flag
                            if (event.key.code == sf::Keyboard::W) FlagSystem::flagEvents.wPressed = isPressed;
                            else FlagSystem::flagEvents.sPressed = isPressed;

                            // Update turns: current player ends, next player begins
                            if (isHost()) {
                                MetaComponents::hostTurn = false;
                                MetaComponents::clientTurn = true;
                            } else {
                                MetaComponents::hostTurn = true;
                                MetaComponents::clientTurn = false;
                            }

                            // Send input with updated turn states
                            std::string keyStr = (event.key.code == sf::Keyboard::W) ? "W" : "S";
                            std::string msgData = keyStr + ":" + (isPressed ? "1" : "0") + ":" + (MetaComponents::hostTurn ? "1" : "0") + ":" + (MetaComponents::clientTurn ? "1" : "0");
                            sendNetworkMessage("INPUT_STATE", msgData);
                        } else if (!isPressed) {
                            // Always send release events
                            if (event.key.code == sf::Keyboard::W) FlagSystem::flagEvents.wPressed = false;
                            else FlagSystem::flagEvents.sPressed = false;

                            std::string keyStr = (event.key.code == sf::Keyboard::W) ? "W" : "S";
                            std::string msgData = keyStr + ":0:" + (MetaComponents::hostTurn ? "1" : "0") + ":" + (MetaComponents::clientTurn ? "1" : "0");
                            sendNetworkMessage("INPUT_STATE", msgData);
                        }
                    } else {
                        // Non-network handling
                        if (event.key.code == sf::Keyboard::W) FlagSystem::flagEvents.wPressed = isPressed; 
                        else FlagSystem::flagEvents.sPressed = isPressed; 
                    }
                    #else
                    // Non-network handling
                    if (event.key.code == sf::Keyboard::W) FlagSystem::flagEvents.wPressed = isPressed; 
                    else FlagSystem::flagEvents.sPressed = isPressed; 
                    #endif
                    break;
                }
  
                case sf::Keyboard::D: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) FlagSystem::flagEvents.dPressed = isPressed; 
                    if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("INPUT_STATE", "D:" + std::string(isPressed ? "1" : "0"));
                    #else
                    FlagSystem::flagEvents.dPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::Space: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) FlagSystem::flagEvents.spacePressed = isPressed; 
                    if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("INPUT_STATE", "SPACE:" + std::string(isPressed ? "1" : "0"));
                    #else
                    FlagSystem::flagEvents.spacePressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::B: FlagSystem::flagEvents.bPressed = isPressed; break;
                case sf::Keyboard::M: FlagSystem::flagEvents.mPressed = isPressed; break;
    
                default: break;
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            #if RUN_NETWORK
            // Calculate mouse position for all cases
            sf::View entireScreenView(sf::FloatRect(0.f, 0.f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT));
            sf::Vector2f worldPosAbsoloute = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), entireScreenView);
            
            // If no network
            if (!isNetworkEnabled) {
                MetaComponents::worldMouseClickedPosition_i = static_cast<sf::Vector2i>(worldPosAbsoloute);
                MetaComponents::worldMouseClickedPosition_f = worldPosAbsoloute; 

                FlagSystem::flagEvents.mouseClicked = true;
                sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::leftView);
                MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
                MetaComponents::leftViewmouseClickedPosition_f = worldPos; 

                worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
                MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
                MetaComponents::middleViewmouseClickedPosition_f = worldPos;
            }
            // Send mouse click over network only if it's our turn
            else if (net.isNetworkConnected() && ((isHost() && MetaComponents::hostTurn) || (!isHost() && MetaComponents::clientTurn))) {
                // Set the flag locally
                MetaComponents::worldMouseClickedPosition_i = static_cast<sf::Vector2i>(worldPosAbsoloute);
                MetaComponents::worldMouseClickedPosition_f = worldPosAbsoloute; 

                FlagSystem::flagEvents.mouseClicked = true;
                sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::leftView);
                MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
                MetaComponents::leftViewmouseClickedPosition_f = worldPos; 

                worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
                MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
                MetaComponents::middleViewmouseClickedPosition_f = worldPos;

                // Update turns: current player ends, next player begins
                if (isHost()) {
                    MetaComponents::hostTurn = false;
                    MetaComponents::clientTurn = true;
                } else {
                    MetaComponents::hostTurn = true;
                    MetaComponents::clientTurn = false;
                }

                // Send mouse click with turn state
                std::string mouseData = std::to_string(worldPosAbsoloute.x) + "," + std::to_string(worldPosAbsoloute.y) + ":" + (MetaComponents::hostTurn ? "1" : "0") + ":" + (MetaComponents::clientTurn ? "1" : "0");
                sendNetworkMessage("MOUSE_CLICK", mouseData);
            }
            #else
            // Non-network version - apply locally
            sf::View entireScreenView(sf::FloatRect(0.f, 0.f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT));
            sf::Vector2f worldPosAbsoloute = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), entireScreenView);
            MetaComponents::worldMouseClickedPosition_i = static_cast<sf::Vector2i>(worldPosAbsoloute);
            MetaComponents::worldMouseClickedPosition_f = worldPosAbsoloute; 

            FlagSystem::flagEvents.mouseClicked = true;
            sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::leftView);
            MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::leftViewmouseClickedPosition_f = worldPos; 

            worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
            MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::middleViewmouseClickedPosition_f = worldPos; 
            #endif
        }
    
        if (!isNetworkEnabled) { // mouse position (no click)
            MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
            MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);
        } else if (net.isNetworkConnected() && ((isHost() && MetaComponents::hostTurn) || (!isHost() && MetaComponents::clientTurn))) {
            MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
            MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);
            std::string mouseData = std::to_string(MetaComponents::middleViewmouseCurrentPosition_f.x) + "," + std::to_string(MetaComponents::middleViewmouseCurrentPosition_f.y);
            sendNetworkMessage("MOUSE_CURRENT", mouseData);
        }

        if(event.type == sf::Event::TextEntered){
            if(event.text.unicode < 128 && event.text.unicode >= 32) {
                char inputChar = static_cast<char>(event.text.unicode);
        
                #if RUN_NETWORK
                if (isHost() || !isNetworkEnabled) MetaComponents::inputText += inputChar;
                if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("TEXT_INPUT", std::string(1, inputChar));
                #else
                MetaComponents::inputText += inputChar;
                #endif

            } else if (event.text.unicode == 8 && !MetaComponents::inputText.empty()) {
                #if RUN_NETWORK
                if (isHost() || !isNetworkEnabled) MetaComponents::inputText.pop_back();
                if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("TEXT_BACKSPACE", "");
                #else
                MetaComponents::inputText.pop_back();
                #endif
            }
        }
    }
} 

void GameManager::resetFlags(){
    FlagSystem::flagEvents.resetFlags();
}

#if RUN_NETWORK
void GameManager::startHosting() {
    if (isNetworkEnabled) return;
    
    if (net.runHost(8080)) {
        isNetworkEnabled = true;
        networkRole = NetworkRole::HOST;
        MetaComponents::hostIP = net.getLocalIP();
        log_info("Starting host on port 8080"); 
    } 
    else log_error("failed to start as host");
}

void GameManager::startClient() {
    if (isNetworkEnabled) return;
    
    if (MetaComponents::inputText != "" && net.runClient(MetaComponents::inputText, 8080)) {
        isNetworkEnabled = true;
        networkRole = NetworkRole::CLIENT;
        FlagSystem::lobby2Events.sceneEnd = true;
        FlagSystem::lobby2Events.sceneStart = false;
        FlagSystem::gameScene1Flags.sceneStart = true;

    } 
}

void GameManager::handleNetworkMessages() {    
    while (net.hasMessages()) {
        NetworkMessage msg = net.getNextMessage();
        processNetworkMessage(msg);
    }
}

void GameManager::processNetworkMessage(const NetworkMessage& msg) {
    if (msg.type == "INPUT_STATE") {
        // Parse input state message: "KEY:STATE:HOSTTURN:CLIENTTURN"
        size_t colonPos = msg.data.find(':');
        if (colonPos != std::string::npos) {
            std::string key = msg.data.substr(0, colonPos);
            std::string remaining = msg.data.substr(colonPos + 1);
            
                if (key == "S" || key == "W") {
                    size_t colon2 = remaining.find(':');
                    size_t colon3 = remaining.find(':', colon2 + 1);
                    
                    if (colon2 != std::string::npos && colon3 != std::string::npos) {
                        bool state = (remaining.substr(0, colon2) == "1");
                        bool hostTurnMsg = (remaining.substr(colon2 + 1, colon3 - colon2 - 1) == "1");
                        bool clientTurnMsg = (remaining.substr(colon3 + 1) == "1");
                        
                        // Apply input state
                        if (key == "S") FlagSystem::flagEvents.sPressed = state;
                        else if (key == "W") FlagSystem::flagEvents.wPressed = state;
                        
                        // Update turn states from press events only
                        if (state) {
                            MetaComponents::hostTurn = hostTurnMsg;
                            MetaComponents::clientTurn = clientTurnMsg;
                        }
                    }
                }else {
                // Handle non-turn-based keys (A, D, SPACE) normally
                size_t colon2 = remaining.find(':');
                std::string stateStr = (colon2 == std::string::npos) ? remaining : remaining.substr(0, colon2);
                bool state = (stateStr == "1");
                
                if (key == "A") FlagSystem::flagEvents.aPressed = state;
                else if (key == "D") FlagSystem::flagEvents.dPressed = state;
                else if (key == "SPACE") FlagSystem::flagEvents.spacePressed = state;
            }
        }
    }

    else if (msg.type == "SCENE_STATE") {
        // Only clients should apply remote scene state
        if (networkRole == NetworkRole::CLIENT) {
            if (msg.data == "LOBBY") {
                // Force lobby scene
                FlagSystem::lobbyEvents.sceneStart = true;
                FlagSystem::lobbyEvents.sceneEnd = false;
                FlagSystem::gameScene1Flags.sceneStart = false;
                FlagSystem::gameScene1Flags.sceneEnd = true;
            } else if (msg.data == "GAME") {
                // Force game scene
                FlagSystem::gameScene1Flags.sceneStart = true;
                FlagSystem::gameScene1Flags.sceneEnd = false;
                FlagSystem::lobbyEvents.sceneStart = false;
                FlagSystem::lobbyEvents.sceneEnd = true;
            }
        }
    }
    else if (msg.type == "GAME_STATE_SYNC") {
        // Only clients should apply remote game state
        if (networkRole == NetworkRole::CLIENT) {
            GameState remoteState = deserializeGameState(msg.data);
            applyRemoteGameState(remoteState);
        }
    }
    else if (msg.type == "MOUSE_CLICK") {
        // Parse the mouse click data with turn information
        size_t firstColon = msg.data.find(':');
        if (firstColon != std::string::npos) {
            // Extract coordinates part
            std::string coordPart = msg.data.substr(0, firstColon);
            std::string turnPart = msg.data.substr(firstColon + 1);
            
            size_t comma = coordPart.find(',');
            if (comma != std::string::npos) {
                float x = std::stof(coordPart.substr(0, comma));
                float y = std::stof(coordPart.substr(comma + 1));

                MetaComponents::worldMouseClickedPosition_f = sf::Vector2f(x, y);
                MetaComponents::worldMouseClickedPosition_i = sf::Vector2i(static_cast<int>(x), static_cast<int>(y));

                // First, get the pixel coordinates on the window for the world click position
                sf::Vector2i pixelPosForWorldClick = mainWindow.getWindow().mapCoordsToPixel(MetaComponents::worldMouseClickedPosition_f, MetaComponents::leftView);
                // Then, convert those pixel coordinates back to world coordinates using the leftView
                MetaComponents::leftViewmouseClickedPosition_f = mainWindow.getWindow().mapPixelToCoords(static_cast<sf::Vector2i>(pixelPosForWorldClick), MetaComponents::leftView);
                MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(MetaComponents::leftViewmouseClickedPosition_f);

                // To get the click position relative to MetaComponents::middleView:
                pixelPosForWorldClick = mainWindow.getWindow().mapCoordsToPixel(MetaComponents::worldMouseClickedPosition_f, MetaComponents::middleView);
                MetaComponents::middleViewmouseClickedPosition_f = mainWindow.getWindow().mapPixelToCoords(static_cast<sf::Vector2i>(pixelPosForWorldClick), MetaComponents::middleView);
                MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseClickedPosition_f);

                FlagSystem::flagEvents.mouseClicked = true;

                // Parse and update turn states
                size_t secondColon = turnPart.find(':');
                if (secondColon != std::string::npos) {
                    MetaComponents::hostTurn = (turnPart.substr(0, secondColon) == "1");
                    MetaComponents::clientTurn = (turnPart.substr(secondColon + 1) == "1");
                }
            }
        }
    } 
    else if (msg.type == "MOUSE_CURRENT") {
        size_t comma = msg.data.find(',');
        if (comma != std::string::npos) {
            float x = std::stof(msg.data.substr(0, comma));
            float y = std::stof(msg.data.substr(comma + 1));

            // To get the position relative to MetaComponents::middleView:
            sf::Vector2i pixelPosForWorldClick = mainWindow.getWindow().mapCoordsToPixel(sf::Vector2f(x, y), MetaComponents::middleView);
            MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(static_cast<sf::Vector2i>(pixelPosForWorldClick), MetaComponents::middleView);
            MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);
        }
    }
    else if (msg.type == "TEXT_INPUT") {
        MetaComponents::inputText += msg.data;
    } 
    else if (msg.type == "TEXT_BACKSPACE") {
        if (!MetaComponents::inputText.empty()) MetaComponents::inputText.pop_back();
    }
    else if (msg.type == "TEXT_STATE_SYNC") {
        if (networkRole == NetworkRole::CLIENT) MetaComponents::inputText = msg.data;
    }
}

void GameManager::sendPlayerInput(const std::string& input) {
    if (isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("PLAYER_INPUT", input);
}

void GameManager::sendNetworkMessage(const std::string& type, const std::string& data) {
    if (!isNetworkEnabled || !net.isNetworkConnected()) return;
    
    NetworkMessage msg;
    msg.type = type;
    msg.sender = (networkRole == NetworkRole::HOST) ? "HOST" : "CLIENT";
    msg.data = data;
    
    net.sendMessage(msg);
}

void GameManager::sendGameState() {
    if (!isNetworkEnabled || !net.isNetworkConnected()) return;
    
    // Create a simple game state string
    std::string gameState = "time:" + std::to_string(MetaComponents::globalTime) + ";mouse:" + std::to_string(MetaComponents::worldMouseClickedPosition_f.x) + "," + std::to_string(MetaComponents::worldMouseClickedPosition_f.y);
    
    sendNetworkMessage("GAME_STATE", gameState);
}

void GameManager::syncGameState() {
    // Update current game state
    currentGameState.globalTime = MetaComponents::globalTime;
    currentGameState.worldMousePos = MetaComponents::worldMouseClickedPosition_f;
    currentGameState.middleViewMousePos = MetaComponents::middleViewmouseClickedPosition_f;
    currentGameState.middleViewMousePosCurr = MetaComponents::middleViewmouseCurrentPosition_f;
    currentGameState.inputText = MetaComponents::inputText;
    
    // Add flag states to sync - use actual current flag states
    currentGameState.flagStates.clear();
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.aPressed);
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.sPressed);
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.wPressed);
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.dPressed);
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.spacePressed);
    currentGameState.flagStates.push_back(FlagSystem::flagEvents.mouseClicked);
    
    // Serialize and send to client
    std::string serializedState = serializeGameState(currentGameState);
    sendNetworkMessage("GAME_STATE_SYNC", serializedState);
}

void GameManager::applyRemoteGameState(const GameState& remoteState) {
    // Apply the remote game state to local game
    MetaComponents::globalTime = remoteState.globalTime;
    MetaComponents::worldMouseClickedPosition_f = remoteState.worldMousePos;
    MetaComponents::middleViewmouseClickedPosition_f = remoteState.middleViewMousePos;
    MetaComponents::middleViewmouseCurrentPosition_f = remoteState.middleViewMousePosCurr;
    MetaComponents::inputText = remoteState.inputText;
    
    // Apply flag states
    if (remoteState.flagStates.size() >= 6) {
        FlagSystem::flagEvents.aPressed = remoteState.flagStates[0];
        FlagSystem::flagEvents.sPressed = remoteState.flagStates[1];
        FlagSystem::flagEvents.wPressed = remoteState.flagStates[2];
        FlagSystem::flagEvents.dPressed = remoteState.flagStates[3];
        FlagSystem::flagEvents.spacePressed = remoteState.flagStates[4];
        FlagSystem::flagEvents.mouseClicked = remoteState.flagStates[5];
    }
}

std::string GameManager::serializeGameState(const GameState& state) {
    std::string serialized;
    serialized += std::to_string(state.globalTime) + ";";
    serialized += std::to_string(state.worldMousePos.x) + "," + std::to_string(state.worldMousePos.y) + ";";
    serialized += std::to_string(state.middleViewMousePos.x) + "," + std::to_string(state.middleViewMousePos.y) + ";";
    serialized += std::to_string(state.middleViewMousePosCurr.x) + "," + std::to_string(state.middleViewMousePosCurr.y) + ";";
    serialized += state.inputText + ";";
    
    // Add flag states
    for (size_t i = 0; i < state.flagStates.size(); ++i) {
        serialized += (state.flagStates[i] ? "1" : "0");
        if (i < state.flagStates.size() - 1) serialized += ",";
    }
    serialized += ";";
    
    // Add player positions
    for (const auto& pos : state.playerPositions) serialized += std::to_string(pos.x) + "," + std::to_string(pos.y) + "|";
    
    return serialized;
}

GameManager::GameState GameManager::deserializeGameState(const std::string& data) {
    GameState state;
    std::stringstream ss(data);
    std::string token;
    
    // Parse global time
    if (std::getline(ss, token, ';')) state.globalTime = std::stof(token);
    
    // Parse world mouse position
    if (std::getline(ss, token, ';')) {
        size_t comma = token.find(',');
        if (comma != std::string::npos) {
            state.worldMousePos.x = std::stof(token.substr(0, comma));
            state.worldMousePos.y = std::stof(token.substr(comma + 1));
        }
    }

    // Parse middle view mouse position
    if (std::getline(ss, token, ';')) {
        size_t comma = token.find(',');
        if (comma != std::string::npos) {
            state.middleViewMousePos.x = std::stof(token.substr(0, comma));
            state.middleViewMousePos.y = std::stof(token.substr(comma + 1));
        }
    }
    
    // Parse middle view mouse position
    if (std::getline(ss, token, ';')) {
        size_t comma = token.find(',');
        if (comma != std::string::npos) {
            state.middleViewMousePosCurr.x = std::stof(token.substr(0, comma));
            state.middleViewMousePosCurr.y = std::stof(token.substr(comma + 1));
        }
    }

    // Parse input text
    if (std::getline(ss, token, ';')) state.inputText = token;
    
    // Parse flag states
    if (std::getline(ss, token, ';')) {
        std::stringstream flagStream(token);
        std::string flagValue;
        while (std::getline(flagStream, flagValue, ',')) state.flagStates.push_back(flagValue == "1");
    }
    
    // Parse player positions (if any)
    if (std::getline(ss, token, ';')) {
        std::stringstream playerStream(token);
        std::string playerPos;
        while (std::getline(playerStream, playerPos, '|')) {
            if (!playerPos.empty()) {
                size_t comma = playerPos.find(',');
                if (comma != std::string::npos) {
                    sf::Vector2f pos;
                    pos.x = std::stof(playerPos.substr(0, comma));
                    pos.y = std::stof(playerPos.substr(comma + 1));
                    state.playerPositions.push_back(pos);
                }
            }
        }
    }
    
    return state;
}

void GameManager::handleSceneSync() {
    if (!isNetworkEnabled || !net.isNetworkConnected()) return;
    
    // Send current scene state
    std::string sceneState = "";
    if (FlagSystem::lobbyEvents.sceneStart && !FlagSystem::lobbyEvents.sceneEnd) sceneState = "LOBBY";
    else if (FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd) sceneState = "GAME";
    
    sendNetworkMessage("SCENE_STATE", sceneState);
}

#endif


// #include "game.hpp" 

// // GameManager constructor sets up the window, intitializes constant variables, calls the random function, and makes scenes 
// GameManager::GameManager()
//     : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT) {
//     introScene = std::make_unique<lobbyScene>(mainWindow.getWindow());
//     gameScene = std::make_unique<gamePlayScene>(mainWindow.getWindow());

//     log_info("\tGame initialized");
// }

// // runGame calls to createAssets from scenes and loops until window is closed to run scene events 
// void GameManager::runGame() {
//     try {     
//         loadScenes(); 

//         while (mainWindow.getWindow().isOpen()) {
//             countTime();
//             handleEventInput();
//             runScenesFlags(); 
//             resetFlags();
//         }
//         log_info("\tGame Ended\n"); 
            
//     } catch (const std::exception& e) {
//         log_error("Exception in runGame: " + std::string(e.what())); 
//         mainWindow.getWindow().close(); 
//     }
// }

// void GameManager::runScenesFlags(){
//     if(FlagSystem::lobbyEvents.sceneStart && !FlagSystem::lobbyEvents.sceneEnd) {
//         introScene->runScene();
//     }
//     if(FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd) gameScene->runScene();
// }

// void GameManager::loadScenes(){
//     introScene->createAssets(); 
//     gameScene->createAssets();
// }

// // countTime counts global time and delta time for scenes to later use in runScene 
// void GameManager::countTime() {
//     sf::Time frameTime = MetaComponents::clock.restart();
//     MetaComponents::deltaTime = frameTime.asSeconds(); 
//     MetaComponents::globalTime += MetaComponents::deltaTime;
// }

// /* handleEventInput takes in keyboard and mouse input. It modifies flagEvents and calls setMouseClickedPos in scene to 
// pass in the position in screen where mouse was clicked */
// void GameManager::handleEventInput() {
//     sf::Event event;
//     while (mainWindow.getWindow().pollEvent(event)) {
//         if (event.type == sf::Event::Closed) {
//             log_info("Window close event detected.");
//             FlagSystem::flagEvents.gameEnd = true;
//             mainWindow.getWindow().close();
//             return; 
//         }
//         if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
//             bool isPressed = (event.type == sf::Event::KeyPressed); 
//             switch (event.key.code) {
//                 case sf::Keyboard::A: FlagSystem::flagEvents.aPressed = isPressed; break;
//                 case sf::Keyboard::S: FlagSystem::flagEvents.sPressed = isPressed; break;
//                 case sf::Keyboard::W: FlagSystem::flagEvents.wPressed = isPressed; break;
//                 case sf::Keyboard::D: FlagSystem::flagEvents.dPressed = isPressed; break;
//                 case sf::Keyboard::B: FlagSystem::flagEvents.bPressed = isPressed; break;
//                 case sf::Keyboard::M: FlagSystem::flagEvents.mPressed = isPressed; break;
//                 case sf::Keyboard::Space: FlagSystem::flagEvents.spacePressed = isPressed; break;
//                 default: break;
//             }
//         }
//         if (event.type == sf::Event::MouseButtonPressed) {
//             sf::View entireScreenView(sf::FloatRect(0.f, 0.f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT)); // left, top, width, height
//             sf::Vector2f worldPosAbsoloute = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), entireScreenView);
//             MetaComponents::worldMouseClickedPosition_i = static_cast<sf::Vector2i>(worldPosAbsoloute);
//             MetaComponents::worldMouseClickedPosition_f = worldPosAbsoloute; 

//             FlagSystem::flagEvents.mouseClicked = true;
//             sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::leftView);
//             MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
//             MetaComponents::leftViewmouseClickedPosition_f = worldPos; 
//             //std::cout << "mouse clicked in big view x: " <<  MetaComponents::leftViewmouseClickedPosition_i.x << " and big view y: " <<  MetaComponents::leftViewmouseClickedPosition_i.y <<std::endl;

//             worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
//             MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
//             MetaComponents::middleViewmouseClickedPosition_f = worldPos; 
//             //std::cout << "mouse clicked in small view x: " <<  MetaComponents::middleViewmouseClickedPosition_i.x << " and small view y: " <<  MetaComponents::middleViewmouseClickedPosition_i.y <<std::endl;
//         }
//         MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
//         MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);

//         if(event.type == sf::Event::TextEntered){
//             if(event.text.unicode < 128 && event.text.unicode >= 32) { // Only process ASCII characters
//                 char inputChar = static_cast<char>(event.text.unicode);
//                 MetaComponents::inputText += inputChar; // Append character to input text            
//             } else if (event.text.unicode == 8 && !MetaComponents::inputText.empty()) {
//                 MetaComponents::inputText.pop_back();
//             }
//         }
//     }
// }

// void GameManager::resetFlags(){
//     FlagSystem::flagEvents.mouseClicked = false;
// }

// Updated game.cpp with networking integration



#include "game.hpp" 

// GameManager constructor sets up the window, initializes constant variables, calls the random function, and makes scenes 
GameManager::GameManager()
    : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT) {
    introScene = std::make_unique<lobbyScene>(mainWindow.getWindow());
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

void GameManager::runGame() {
    try {     
        loadScenes(); 

        while (mainWindow.getWindow().isOpen()) {
            countTime();
            handleEventInput();
            
            #if RUN_NETWORK
            handleNetworkMessages();
            
            // Only sync at specified intervals to reduce network spam
            if (isHost() && isNetworkEnabled && net.isNetworkConnected()) {
                if (MetaComponents::globalTime - lastSyncTime >= syncInterval) {
                    syncGameState();
                    lastSyncTime = MetaComponents::globalTime;
                }
            }
            #endif
            
            runScenesFlags(); 
            resetFlags();
        }
        log_info("\tGame Ended\n"); 
            
    } catch (const std::exception& e) {
        log_error("Exception in runGame: " + std::string(e.what())); 
        mainWindow.getWindow().close(); 
    }
}

void GameManager::runScenesFlags(){
    // Check for scene transitions and sync if we're the host
    static bool wasInLobby = false;
    static bool wasInGame = false;
    
    bool currentlyInLobby = FlagSystem::lobbyEvents.sceneStart && !FlagSystem::lobbyEvents.sceneEnd;
    bool currentlyInGame = FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd;
    
    // Detect scene changes
    if (currentlyInLobby != wasInLobby || currentlyInGame != wasInGame) {
        #if RUN_NETWORK
        if (isHost() && isNetworkEnabled && net.isNetworkConnected()) handleSceneSync(); // Send scene change immediately
        #endif
    }
    
    // Update previous states
    wasInLobby = currentlyInLobby;
    wasInGame = currentlyInGame;
    
    // Run the actual scenes
    if(currentlyInLobby) introScene->runScene();
    if(currentlyInGame) gameScene->runScene();
}

void GameManager::loadScenes(){
    introScene->createAssets(); 
    gameScene->createAssets();
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
            return; 
        }
        
        if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
            bool isPressed = (event.type == sf::Event::KeyPressed); 
            switch (event.key.code) {
                case sf::Keyboard::A: 
                    #if RUN_NETWORK
                    // Host: Apply input locally
                    if (isHost() || !isNetworkEnabled) {
                        FlagSystem::flagEvents.aPressed = isPressed; 
                    }
                    // Everyone: Send input state over network
                    if (isNetworkEnabled && net.isNetworkConnected()) {
                        sendNetworkMessage("INPUT_STATE", "A:" + std::string(isPressed ? "1" : "0"));
                    }
                    #else
                    FlagSystem::flagEvents.aPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::S: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) {
                        FlagSystem::flagEvents.sPressed = isPressed; 
                    }
                    if (isNetworkEnabled && net.isNetworkConnected()) {
                        sendNetworkMessage("INPUT_STATE", "S:" + std::string(isPressed ? "1" : "0"));
                    }
                    #else
                    FlagSystem::flagEvents.sPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::W: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) {
                        FlagSystem::flagEvents.wPressed = isPressed; 
                    }
                    if (isNetworkEnabled && net.isNetworkConnected()) {
                        sendNetworkMessage("INPUT_STATE", "W:" + std::string(isPressed ? "1" : "0"));
                    }
                    #else
                    FlagSystem::flagEvents.wPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::D: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) {
                        FlagSystem::flagEvents.dPressed = isPressed; 
                    }
                    if (isNetworkEnabled && net.isNetworkConnected()) {
                        sendNetworkMessage("INPUT_STATE", "D:" + std::string(isPressed ? "1" : "0"));
                    }
                    #else
                    FlagSystem::flagEvents.dPressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::Space: 
                    #if RUN_NETWORK
                    if (isHost() || !isNetworkEnabled) {
                        FlagSystem::flagEvents.spacePressed = isPressed; 
                    }
                    if (isNetworkEnabled && net.isNetworkConnected()) {
                        sendNetworkMessage("INPUT_STATE", "SPACE:" + std::string(isPressed ? "1" : "0"));
                    }
                    #else
                    FlagSystem::flagEvents.spacePressed = isPressed; 
                    #endif
                    break;
                    
                case sf::Keyboard::B: FlagSystem::flagEvents.bPressed = isPressed; break;
                case sf::Keyboard::M: FlagSystem::flagEvents.mPressed = isPressed; break;
                    
                #if RUN_NETWORK
                case sf::Keyboard::H: 
                    if (isPressed) startHosting();
                    break;
                case sf::Keyboard::J: 
                    if (isPressed) startClient();
                    break;
                #endif
                default: break;
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            #if RUN_NETWORK
            // Calculate mouse position for all cases
            sf::View entireScreenView(sf::FloatRect(0.f, 0.f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT));
            sf::Vector2f worldPosAbsoloute = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), entireScreenView);
            
            // If no network OR we're the host, apply locally
            if (!isNetworkEnabled || isHost()) {
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
            
            // Send mouse click over network (both host and client can send)
            if (isNetworkEnabled && net.isNetworkConnected()) {
                std::string mouseData = std::to_string(worldPosAbsoloute.x) + "," + 
                                       std::to_string(worldPosAbsoloute.y);
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
        
        MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
        MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);

        if(event.type == sf::Event::TextEntered){
            if(event.text.unicode < 128 && event.text.unicode >= 32) {
                char inputChar = static_cast<char>(event.text.unicode);
        
                #if RUN_NETWORK
                if (isHost() || !isNetworkEnabled) {
                    MetaComponents::inputText += inputChar;
                }
                if (isNetworkEnabled && net.isNetworkConnected()) {
                    sendNetworkMessage("TEXT_INPUT", std::string(1, inputChar));
                }
                #else
                MetaComponents::inputText += inputChar;
                #endif
                    
            } else if (event.text.unicode == 8 && !MetaComponents::inputText.empty()) {
                    
                #if RUN_NETWORK
                if (isHost() || !isNetworkEnabled) {
                    MetaComponents::inputText.pop_back();
                }
                if (isNetworkEnabled && net.isNetworkConnected()) {
                    sendNetworkMessage("TEXT_BACKSPACE", "");
                }
                #else
                MetaComponents::inputText.pop_back();
                #endif
            }
        }
    }
}

void GameManager::resetFlags(){
    FlagSystem::flagEvents.mouseClicked = false;
}

#if RUN_NETWORK
void GameManager::startHosting() {
    if (isNetworkEnabled) return;
    
    std::cout << "Starting host on port 8080..." << std::endl;
    if (net.runHost(8080)) {
        isNetworkEnabled = true;
        networkRole = NetworkRole::HOST;
        std::cout << "Host started successfully!" << std::endl;
        std::cout << "Share your IP: " << net.getLocalIP() << std::endl;
    } else {
        std::cout << "Failed to start host!" << std::endl;
    }
}

void GameManager::startClient() {
    if (isNetworkEnabled) return;
    
    // Auto-use localhost for testing
    std::string hostIP = "127.0.0.1";
    std::cout << "Connecting to localhost (127.0.0.1):8080..." << std::endl;
    
    if (net.runClient(hostIP, 8080)) {
        isNetworkEnabled = true;
        networkRole = NetworkRole::CLIENT;
        std::cout << "Connected successfully!" << std::endl;
    } else {
        std::cout << "Failed to connect!" << std::endl;
    }
}

void GameManager::handleNetworkMessages() {
    if (!isNetworkEnabled) return;
    
    while (net.hasMessages()) {
        NetworkMessage msg = net.getNextMessage();
        processNetworkMessage(msg);
    }
}

void GameManager::processNetworkMessage(const NetworkMessage& msg) {
    // Reduce debug output to avoid spam
    if (msg.type != "GAME_STATE_SYNC") {
        std::cout << "Received: " << msg.type << " from " << msg.sender << ": " << msg.data << std::endl;
    }
    
    if (msg.type == "INPUT_STATE") {
        // Parse input state message: "KEY:STATE"
        size_t colonPos = msg.data.find(':');
        if (colonPos != std::string::npos) {
            std::string key = msg.data.substr(0, colonPos);
            bool state = (msg.data.substr(colonPos + 1) == "1");
            
            // Apply the input state
            if (key == "A") {
                FlagSystem::flagEvents.aPressed = state;
            } else if (key == "S") {
                FlagSystem::flagEvents.sPressed = state;
            } else if (key == "W") {
                FlagSystem::flagEvents.wPressed = state;
            } else if (key == "D") {
                FlagSystem::flagEvents.dPressed = state;
            } else if (key == "SPACE") {
                FlagSystem::flagEvents.spacePressed = state;
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
        // Parse and apply the mouse click position
        size_t comma = msg.data.find(',');
        if (comma != std::string::npos) {
            float x = std::stof(msg.data.substr(0, comma));
            float y = std::stof(msg.data.substr(comma + 1));
            
            // Apply mouse click to all views (both host and client should apply received clicks)
            MetaComponents::worldMouseClickedPosition_f = sf::Vector2f(x, y);
            MetaComponents::worldMouseClickedPosition_i = sf::Vector2i(static_cast<int>(x), static_cast<int>(y));
            
            // Calculate positions for other views based on world position
            // Note: This is a simplified approach - you might need to adjust based on your view setup
            sf::Vector2f worldPos = sf::Vector2f(x, y);
            
            // Convert world position to left view coordinates
            MetaComponents::leftViewmouseClickedPosition_f = worldPos; // Adjust transformation as needed
            MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(MetaComponents::leftViewmouseClickedPosition_f);
            
            // Convert world position to middle view coordinates
            MetaComponents::middleViewmouseClickedPosition_f = worldPos; // Adjust transformation as needed
            MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseClickedPosition_f);
            
            FlagSystem::flagEvents.mouseClicked = true;
        }
    } 
    else if (msg.type == "TEXT_INPUT") {
        MetaComponents::inputText += msg.data;
    } 
    else if (msg.type == "TEXT_BACKSPACE") {
        if (!MetaComponents::inputText.empty()) {
            MetaComponents::inputText.pop_back();
        }
    }
    else if (msg.type == "TEXT_STATE_SYNC") {
        if (networkRole == NetworkRole::CLIENT) {
            MetaComponents::inputText = msg.data;
        }
    }
}

void GameManager::sendPlayerInput(const std::string& input) {
    if (isNetworkEnabled && net.isNetworkConnected()) {
        sendNetworkMessage("PLAYER_INPUT", input);
    }
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
    std::string gameState = "time:" + std::to_string(MetaComponents::globalTime) + 
                           ";mouse:" + std::to_string(MetaComponents::worldMouseClickedPosition_f.x) + 
                           "," + std::to_string(MetaComponents::worldMouseClickedPosition_f.y);
    
    sendNetworkMessage("GAME_STATE", gameState);
}

void GameManager::testNetworkInGame() {
    std::cout << "\n=== NETWORK TEST IN GAME ===" << std::endl;
    std::cout << "Press H to host, J to join as client" << std::endl;
    std::cout << "Use WASD keys to test input sharing" << std::endl;
    std::cout << "Click mouse to test mouse sharing" << std::endl;
    std::cout << "Type text to test text sharing" << std::endl;
    std::cout << "Network status will show in console" << std::endl;
}

void GameManager::syncGameState() {
    // Update current game state
    currentGameState.globalTime = MetaComponents::globalTime;
    currentGameState.worldMousePos = MetaComponents::worldMouseClickedPosition_f;
    currentGameState.middleViewMousePos = MetaComponents::middleViewmouseCurrentPosition_f;
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
    MetaComponents::middleViewmouseCurrentPosition_f = remoteState.middleViewMousePos;
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
    serialized += state.inputText + ";";
    
    // Add flag states
    for (size_t i = 0; i < state.flagStates.size(); ++i) {
        serialized += (state.flagStates[i] ? "1" : "0");
        if (i < state.flagStates.size() - 1) serialized += ",";
    }
    serialized += ";";
    
    // Add player positions
    for (const auto& pos : state.playerPositions) {
        serialized += std::to_string(pos.x) + "," + std::to_string(pos.y) + "|";
    }
    
    return serialized;
}

GameManager::GameState GameManager::deserializeGameState(const std::string& data) {
    GameState state;
    std::stringstream ss(data);
    std::string token;
    
    // Parse global time
    if (std::getline(ss, token, ';')) {
        state.globalTime = std::stof(token);
    }
    
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
    
    // Parse input text
    if (std::getline(ss, token, ';')) {
        state.inputText = token;
    }
    
    // Parse flag states
    if (std::getline(ss, token, ';')) {
        std::stringstream flagStream(token);
        std::string flagValue;
        while (std::getline(flagStream, flagValue, ',')) {
            state.flagStates.push_back(flagValue == "1");
        }
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
    if (FlagSystem::lobbyEvents.sceneStart && !FlagSystem::lobbyEvents.sceneEnd) {
        sceneState = "LOBBY";
    } else if (FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd) {
        sceneState = "GAME";
    }
    
    sendNetworkMessage("SCENE_STATE", sceneState);
}

#endif
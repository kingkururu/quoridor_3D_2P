#include "game.hpp" 

// GameManager constructor sets up the window, intitializes constant variables, calls the random function, and makes scenes 
GameManager::GameManager()
    : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT), net() {
    introScene = std::make_unique<lobbyScene>(mainWindow.getWindow());
    introScene2 = std::make_unique<lobby2Scene>(mainWindow.getWindow());

    gameScene = std::make_unique<gamePlayScene>(mainWindow.getWindow());

    log_info("\tGame initialized");
}

GameManager::~GameManager() {
    net.cleanup(); 
    log_info("GameManager destructor called");
}

// runGame calls to createAssets from scenes and loops until window is closed to run scene events 
void GameManager::runGame() {
    try {     
        loadScenes(); 

        while (mainWindow.getWindow().isOpen()) {
            countTime();
            handleEventInput();
            runScenesFlags(); 
            resetFlags();
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
    if (currentlyInLobby1 && introScene) introScene->runScene();
    else if (currentlyInLobby2 && introScene2) introScene2->runScene();
    else if (currentlyInGame && gameScene) gameScene->runScene();

    if (currentlyInLobby2) {
        MetaComponents::hostIP = net.getLocalIP();

        // === HOST GAME BUTTON CLICKED ===
        if (FlagSystem::lobby2Events.hostButtonClicked && net.isInLobby()) {
            if (net.startServer(8080)) {
                std::cout << "=================================\n";
                std::cout << "GAME CODE: " << net.getGameCode() << std::endl;
                std::cout << "Share this code with other players!\n";
                std::cout << "Waiting for players to join...\n";
                std::cout << "=================================\n";
                MetaComponents::gameCode = net.getGameCode();
            } else {
                std::cout << "Failed to start server!\n";
            }
        }

        // === HOSTING LOGIC ===
        if (net.isHosting()) {
            // Try to accept new clients
            if (net.acceptClient()) {
                std::cout << "A player has joined your game!\n";
                net.sendMessage("Welcome to the game!");
            }
            
            // Handle messages from clients
            std::string message = net.receiveMessage();
            if (!message.empty()) {
                std::cout << "Client: " << message << std::endl;
                // Echo back or handle the message
                net.sendMessage("Server received: " + message);
            }
        }

        // === JOIN GAME LOGIC ===
        if (net.isInLobby() && FlagSystem::flagEvents.enterPressed && MetaComponents::inputText.length() == 6) {
            std::cout << "Attempting to join game with code: " << MetaComponents::inputText << std::endl;
            
            if (net.connectToServer(MetaComponents::inputText, 8080)) {
                std::cout << "Successfully joined the game!\n";
                net.sendMessage("Hello from client!");
                MetaComponents::inputText.clear();
            } else {
                std::cout << "Failed to join game. Check the code and try again.\n";
                MetaComponents::inputText.clear();
            }
        }
        
        // === CLIENT LOGIC ===
        if (net.isClient()) {
            // Handle messages from server
            std::string message = net.receiveMessage();
            if (!message.empty()) {
                std::cout << "Server: " << message << std::endl;
            }
        }
        
        // === INPUT VALIDATION ===
        if (net.isInLobby() && FlagSystem::flagEvents.enterPressed && 
            MetaComponents::inputText.length() > 0 && MetaComponents::inputText.length() < 6) {
            std::cout << "Game code must be exactly 6 digits. Current: " << MetaComponents::inputText << std::endl;
        }

        // === STATUS UPDATES (Optional - for debugging) ===
        static float lastStatusUpdate = 0;
        if (MetaComponents::globalTime - lastStatusUpdate > 3.0f) {
            lastStatusUpdate = MetaComponents::globalTime;
            
            if (net.isHosting()) {
                std::cout << "[STATUS] Hosting game - Code: " << net.getGameCode() << std::endl;
            } else if (net.isClient()) {
                std::cout << "[STATUS] Connected to game as client" << std::endl;
            } else if (MetaComponents::inputText.empty()) {
                std::cout << "[STATUS] In lobby - Host a game or enter code to join" << std::endl;
            }
        }
    }
}

void GameManager::loadScenes(){
    introScene->createAssets(); 
    introScene2->createAssets(); 
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
                case sf::Keyboard::A: FlagSystem::flagEvents.aPressed = isPressed; break;
                case sf::Keyboard::S: FlagSystem::flagEvents.sPressed = isPressed; break;
                case sf::Keyboard::W: FlagSystem::flagEvents.wPressed = isPressed; break;
                case sf::Keyboard::D: FlagSystem::flagEvents.dPressed = isPressed; break;
                case sf::Keyboard::B: FlagSystem::flagEvents.bPressed = isPressed; break;
                case sf::Keyboard::M: FlagSystem::flagEvents.mPressed = isPressed; break;
                case sf::Keyboard::Space: FlagSystem::flagEvents.spacePressed = isPressed; break;
                case sf::Keyboard::Enter: 
                    if (isPressed) {
                        FlagSystem::flagEvents.enterPressed = true;
                    }
                    break;
                case sf::Keyboard::Escape:
                    if (isPressed) {
                        // Clear input text on Escape
                        MetaComponents::inputText.clear();
                    }
                    break;
                default: break;
            }
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
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
        }
        
        MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
        MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);

        // FIXED TEXT INPUT HANDLING
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode < 128 && event.text.unicode >= 32) { // Only process ASCII characters
                char inputChar = static_cast<char>(event.text.unicode);
                
                // Only allow digits for game code input and limit to 6 characters
                if (std::isdigit(inputChar) && MetaComponents::inputText.length() < 6) {
                    MetaComponents::inputText += inputChar;
                    std::cout << "Game code input: " << MetaComponents::inputText << std::endl;
                }
                // If they try to enter non-digit, give feedback
                else if (!std::isdigit(inputChar)) {
                    std::cout << "Game codes only contain numbers (0-9)" << std::endl;
                }
                // If they try to enter more than 6 digits
                else if (MetaComponents::inputText.length() >= 6) {
                    std::cout << "Game code complete: " << MetaComponents::inputText << " (Press Enter to join)" << std::endl;
                }
            } 
            // Handle backspace
            else if (event.text.unicode == 8 && !MetaComponents::inputText.empty()) {
                MetaComponents::inputText.pop_back();
                std::cout << "Game code input: " << MetaComponents::inputText << std::endl;
            }
        }
    }
}

void GameManager::resetFlags(){
    FlagSystem::flagEvents.mouseClicked = false;
    FlagSystem::flagEvents.enterPressed = false; // Add this new flag
}

// void GameManager::resetFlags(){
//     FlagSystem::flagEvents.mouseClicked = false;
// }
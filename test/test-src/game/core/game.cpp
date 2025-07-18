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

        if (FlagSystem::lobby2Events.hostButtonClicked && !hostServerStarted) {
            hostServerStarted = true;
            net.startServer(8080);
        }

        if (hostServerStarted && !net.isServerConnected()) {
            net.acceptClient(); // new function that tries to accept clients
        }

        if (!clientServerJoined && MetaComponents::inputText != "") {
            clientServerJoined = net.connectToServer(MetaComponents::inputText, 8080);
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

/* handleEventInput takes in keyboard and mouse input. It modifies flagEvents and calls setMouseClickedPos in scene to 
pass in the position in screen where mouse was clicked */
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
                default: break;
            }
        }
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::View entireScreenView(sf::FloatRect(0.f, 0.f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT)); // left, top, width, height
            sf::Vector2f worldPosAbsoloute = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), entireScreenView);
            MetaComponents::worldMouseClickedPosition_i = static_cast<sf::Vector2i>(worldPosAbsoloute);
            MetaComponents::worldMouseClickedPosition_f = worldPosAbsoloute; 

            FlagSystem::flagEvents.mouseClicked = true;
            sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::leftView);
            MetaComponents::leftViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::leftViewmouseClickedPosition_f = worldPos; 
            //std::cout << "mouse clicked in big view x: " <<  MetaComponents::leftViewmouseClickedPosition_i.x << " and big view y: " <<  MetaComponents::leftViewmouseClickedPosition_i.y <<std::endl;

            worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
            MetaComponents::middleViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::middleViewmouseClickedPosition_f = worldPos; 
            //std::cout << "mouse clicked in small view x: " <<  MetaComponents::middleViewmouseClickedPosition_i.x << " and small view y: " <<  MetaComponents::middleViewmouseClickedPosition_i.y <<std::endl;
        }
        MetaComponents::middleViewmouseCurrentPosition_f = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::middleView);
        MetaComponents::middleViewmouseCurrentPosition_i = static_cast<sf::Vector2i>(MetaComponents::middleViewmouseCurrentPosition_f);

        if(event.type == sf::Event::TextEntered){
            if(event.text.unicode < 128 && event.text.unicode >= 32) { // Only process ASCII characters
                char inputChar = static_cast<char>(event.text.unicode);
                MetaComponents::inputText += inputChar; // Append character to input text            
            } else if (event.text.unicode == 8 && !MetaComponents::inputText.empty()) {
                MetaComponents::inputText.pop_back();
            }
        }
    }
}

void GameManager::resetFlags(){
    FlagSystem::flagEvents.mouseClicked = false;
}
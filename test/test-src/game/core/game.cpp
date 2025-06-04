
#include "game.hpp" 

// GameManager constructor sets up the window, intitializes constant variables, calls the random function, and makes scenes 
GameManager::GameManager()
    : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT) {
    gameScene = std::make_unique<gamePlayScene>(mainWindow.getWindow());

    log_info("\tGame initialized");
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

void GameManager::runScenesFlags(){
    if(!FlagSystem::flagEvents.gameEnd){
        if(FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameScene1Flags.sceneEnd) gameScene->runScene();
    }
    
}

void GameManager::loadScenes(){
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
    }
}

void GameManager::resetFlags(){
    FlagSystem::flagEvents.mouseClicked = false;
}



#include "game.hpp" 

// GameManager constructor sets up the window, intitializes constant variables, calls the random function, and makes scenes 
GameManager::GameManager()
    : mainWindow(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT, Constants::GAME_TITLE, Constants::FRAME_LIMIT) {
    introScreenScene = std::make_unique<introScene>(mainWindow.getWindow());
    gameScene = std::make_unique<gamePlayScene>(mainWindow.getWindow());
    gameSceneNext = std::make_unique<gamePlayScene2>(mainWindow.getWindow()); 

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
        if(FlagSystem::gameScene1Flags.sceneStart && !FlagSystem::gameSceneNextFlags.sceneStart) gameScene->runScene();

        if(FlagSystem::gameSceneNextFlags.sceneStart && !FlagSystem::gameSceneNextFlags.sceneEnd) gameSceneNext->runScene();
    }
}

void GameManager::loadScenes(){
    introScreenScene->createAssets(); 
    gameScene->createAssets();
    gameSceneNext->createAssets(); 
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
        if (event.type == sf::Event::Resized){ 
            float aspectRatio = static_cast<float>(event.size.width) / event.size.height;
            sf::FloatRect visibleArea(0.0f, 0.0f, Constants::VIEW_SIZE_X, Constants::VIEW_SIZE_X / aspectRatio);
            // MetaComponents::smallView = sf::View(visibleArea); 
            // //keeping this makes the small view in proportion but be cut out sometimes if resized
            // MetaComponents::smallView.setViewport(sf::FloatRect(0.75f, 0.f, 0.25f, 0.25f));

            MetaComponents::bigView = sf::View(visibleArea); 
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
            sf::Vector2f worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::bigView);
            MetaComponents::bigViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::bigViewmouseClickedPosition_f = worldPos; 
            std::cout << "big view x: " <<  MetaComponents::bigViewmouseClickedPosition_i.x << " and big view y: " <<  MetaComponents::bigViewmouseClickedPosition_i.y <<std::endl;

            worldPos = mainWindow.getWindow().mapPixelToCoords(sf::Mouse::getPosition(mainWindow.getWindow()), MetaComponents::smallView);
            MetaComponents::smallViewmouseClickedPosition_i = static_cast<sf::Vector2i>(worldPos);
            MetaComponents::smallViewmouseClickedPosition_f = worldPos; 
            std::cout << "small view x: " <<  MetaComponents::smallViewmouseClickedPosition_i.x << " and small view y: " <<  MetaComponents::smallViewmouseClickedPosition_i.y <<std::endl;
        }
    }
}

void GameManager::resetFlags(){
    FlagSystem::flagEvents.mouseClicked = false;
}


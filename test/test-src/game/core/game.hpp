#pragma once

#include <iostream>
#include <stdexcept>
#include <SFML/Graphics.hpp>
#include "../scenes/scenes.hpp"
#include "network.hpp"

class GameManager {
public:
    GameManager();
    ~GameManager();
    
    void loadScenes();
    void runGame();
    void runScenesFlags();
    void resetFlags();

private:
    void countTime();
    void handleEventInput();
    GameWindow mainWindow;
    std::unique_ptr<gamePlayScene> gameScene;
    std::unique_ptr<lobbyScene> introScene; // lobby
    std::unique_ptr<lobby2Scene> introScene2; 

    NetworkManager net; 

    bool hostServerStarted {};
    bool clientServerJoined {};
};
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

    #if RUN_NETWORK
    NetworkManager net;
    bool isNetworkEnabled;
    NetworkRole networkRole;
    float lastSyncTime;
    float syncInterval;
    std::string authorizedInputText; // Host's authoritative text
    
    struct GameState {
        float globalTime;
        sf::Vector2f worldMousePos;
        sf::Vector2f middleViewMousePos;
        std::string inputText;
        std::vector<bool> flagStates;  // Store flag states for synchronization
        std::vector<sf::Vector2f> playerPositions;
    };
    GameState currentGameState;
    
    bool isHost() const { return networkRole == NetworkRole::HOST; }
    void syncGameState();
    void applyRemoteGameState(const GameState& remoteState);
    std::string serializeGameState(const GameState& state);
    GameState deserializeGameState(const std::string& data);
    void startHosting();
    void startClient();
    void handleNetworkMessages();
    void processNetworkMessage(const NetworkMessage& msg);
    void sendPlayerInput(const std::string& input);
    void sendNetworkMessage(const std::string& type, const std::string& data);
    void sendGameState();
    void handleSceneSync();
    void syncTextState() { if (isHost() && isNetworkEnabled && net.isNetworkConnected()) sendNetworkMessage("TEXT_STATE_SYNC", MetaComponents::inputText); }
    #endif
};
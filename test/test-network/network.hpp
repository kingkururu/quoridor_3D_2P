#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <iostream>
#include <random>
#include <unordered_map>
#include <chrono>
#include "log.hpp"

enum class NetworkRole {
    NONE,
    SERVER,
    CLIENT
};

class NetworkManager {
private:
    sf::TcpListener listener;
    sf::TcpSocket socket;
    NetworkRole role;
    std::string gameCode;
    
    std::string generateGameCode();

public:
    NetworkManager();
    ~NetworkManager();

    bool startServer(unsigned short port = 8080);
    bool connectToServer(const std::string& gameCode, unsigned short port = 8080);
    
    void sendMessage(const std::string& message);
    std::string receiveMessage();
    
    NetworkRole getRole() const;
    std::string getLocalIP() const;
    std::string getGameCode() const;
    
    bool acceptClient();
    bool isServerConnected() const;
    
    // Helper methods for clearer state checking
    bool isHosting() const { return role == NetworkRole::SERVER; }
    bool isClient() const { return role == NetworkRole::CLIENT; }
    bool isInLobby() const { return role == NetworkRole::NONE; }
    bool hasActiveConnection() const { return role != NetworkRole::NONE; }
    
    void cleanup(); // Clean up resources and remove game code
};

class GameCodeRegistry {
private:
    GameCodeRegistry() = default;

    static GameCodeRegistry* instance;
    std::unordered_map<std::string, std::string> activeGameCodes;
    std::mutex codesMutex;
        
public:
    static GameCodeRegistry& getInstance() { static GameCodeRegistry instance; return instance; }
    
    bool addGameCode(const std::string& code, const std::string& ip);
    
    std::string getIPForCode(const std::string& code);
    
    void removeGameCode(const std::string& code);
    
    std::vector<std::string> getAllCodes();
};
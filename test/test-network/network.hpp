#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <iostream>

#include "log.hpp"

enum class NetworkRole {
    NONE,
    SERVER,
    CLIENT
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool startServer(unsigned short port);
    bool connectToServer(const std::string& ip, unsigned short port);

    void sendMessage(const std::string& message);
    std::string receiveMessage();

    NetworkRole getRole() const;
    std::string getLocalIP() const;

    bool acceptClient();
    bool isServerConnected() const;
    
private:
    sf::TcpListener listener;
    sf::TcpSocket socket;
    NetworkRole role;
};

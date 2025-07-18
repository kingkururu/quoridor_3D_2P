#include "network.hpp"

NetworkManager::NetworkManager() : role(NetworkRole::NONE) {}

NetworkManager::~NetworkManager() {}

bool NetworkManager::startServer(unsigned short port) {
    if (listener.listen(port) != sf::Socket::Done) {
        log_error("Server failed to listen on port " + std::to_string(port));
        return false;
    }

    listener.setBlocking(false);  // So we can accept non-blocking later
    role = NetworkRole::SERVER;
    std::cout << "Server started. Waiting for client to connect...\n";
    return true;
}

bool NetworkManager::connectToServer(const std::string& ip, unsigned short port) {
    socket.setBlocking(true);
    if (socket.connect(ip, port) != sf::Socket::Done) {
        log_error("Client failed to connect to server");
        return false;
    }
    role = NetworkRole::CLIENT;
    log_info("Connected to server at " + ip);
    return true;
}

void NetworkManager::sendMessage(const std::string& message) {
    sf::Packet packet;
    packet << message;
    if (socket.send(packet) != sf::Socket::Done) {
        log_error("Failed to send message");
    }
}

std::string NetworkManager::receiveMessage() {
    sf::Packet packet;
    std::string message;
    if (socket.receive(packet) != sf::Socket::Done) {
        log_error("Failed to receive message");
        return "";
    }
    packet >> message;
    return message;
}

NetworkRole NetworkManager::getRole() const {
    return role;
}

std::string NetworkManager::getLocalIP() const {
    sf::IpAddress localIP = sf::IpAddress::getLocalAddress();
    if (localIP == sf::IpAddress::None || localIP == sf::IpAddress::LocalHost) return "127.0.0.1"; // default
    return localIP.toString();
}

bool NetworkManager::acceptClient() {
    if (listener.accept(socket) == sf::Socket::Done) {
        role = NetworkRole::SERVER;
        log_info("Client connected successfully.");
        return true;
    }
    return false;
}

bool NetworkManager::isServerConnected() const {
    return role == NetworkRole::SERVER;
}

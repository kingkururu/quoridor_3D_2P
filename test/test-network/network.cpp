#include "network.hpp"
#include <iostream>

NetworkManager::NetworkManager() : role(NetworkRole::NONE), gameCode(""){}

NetworkManager::~NetworkManager() {
    cleanup();
}

std::string NetworkManager::generateGameCode() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999); // 6-digit range
    
    std::string code;
    int attempts = 0;
    const int maxAttempts = 100;
    
    do {
        code = std::to_string(dis(gen));
        attempts++;
    } while (GameCodeRegistry::getInstance().getIPForCode(code) != "" && attempts < maxAttempts);
    
    if (attempts >= maxAttempts) {
        log_error("Failed to generate unique game code after " + std::to_string(maxAttempts) + " attempts");
        return "";
    }
    
    return code;
}

bool NetworkManager::startServer(unsigned short port) {
    // First try to start the listener
    if (listener.listen(port) != sf::Socket::Done) {
        log_error("Server failed to listen on port " + std::to_string(port));
        return false;
    }

    listener.setBlocking(false);  // So we can accept non-blocking later
    
    // Generate unique game code
    gameCode = generateGameCode();
    if (gameCode.empty()) {
        log_error("Failed to generate game code");
        listener.close();
        return false;
    }
    
    // Get local IP
    std::string localIP = getLocalIP();
    std::cout << "DEBUG: Local IP determined as: " << localIP << std::endl;
    
    // Add the game code to registry
    if (!GameCodeRegistry::getInstance().addGameCode(gameCode, localIP)) {
        log_error("Failed to register game code");
        listener.close();
        return false;
    }
    
    // Set role after successful setup
    role = NetworkRole::SERVER;
    
    std::cout << "Server started. Game Code: " << gameCode << std::endl;
    std::cout << "Players can join using this code!\n";
    
    log_info("Server started with game code: " + gameCode + " on IP: " + localIP);
    return true;
}

bool NetworkManager::connectToServer(const std::string& inputCode, unsigned short port) {
    std::cout << "DEBUG: Attempting to connect with code: " << inputCode << std::endl;
    
    // Look up IP address from game code
    std::string serverIP = GameCodeRegistry::getInstance().getIPForCode(inputCode);
    std::cout << serverIP;
    
    if (serverIP.empty()) {
        log_error("Invalid game code: " + inputCode);
        std::cout << "Invalid game code. Please check the code and try again.\n";
        std::cout << "Available codes: ";
        auto codes = GameCodeRegistry::getInstance().getAllCodes();
        for (const auto& code : codes) {
            std::cout << code << " ";
        }
        std::cout << std::endl;
        return false;
    }
    
    std::cout << "DEBUG: Found IP " << serverIP << " for code " << inputCode << std::endl;
    
    socket.setBlocking(true);
    if (socket.connect(serverIP, port) != sf::Socket::Done) {
        log_error("Client failed to connect to server at " + serverIP);
        std::cout << "Failed to connect to game. The host might be unavailable.\n";
        return false;
    }
    
    role = NetworkRole::CLIENT;
    log_info("Connected to server using game code: " + inputCode + " (IP: " + serverIP + ")");
    std::cout << "Successfully connected to game!\n";
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
    sf::Socket::Status status = socket.receive(packet);
    
    if (status == sf::Socket::Done) {
        packet >> message;
        return message;
    } else if (status == sf::Socket::NotReady) {
        // No message available right now, this is normal for non-blocking
        return "";
    } else {
        log_error("Failed to receive message");
        return "";
    }
}

NetworkRole NetworkManager::getRole() const {
    return role;
}

std::string NetworkManager::getLocalIP() const {
    sf::IpAddress localIP = sf::IpAddress::getLocalAddress();
    if (localIP == sf::IpAddress::None || localIP == sf::IpAddress::LocalHost) 
        return "127.0.0.1"; // default
    return localIP.toString();
}

bool NetworkManager::acceptClient() {
    if (listener.accept(socket) == sf::Socket::Done) {
        socket.setBlocking(false); // Set client socket to non-blocking
        log_info("Client connected successfully using game code: " + gameCode);
        std::cout << "Player joined the game!\n";
        return true;
    }
    return false;
}

bool NetworkManager::isServerConnected() const {
    return (role == NetworkRole::SERVER || role == NetworkRole::CLIENT);
}

std::string NetworkManager::getGameCode() const {
    return gameCode;
}

void NetworkManager::cleanup() {
    // Clean up game code from active codes map
    if (!gameCode.empty() && role == NetworkRole::SERVER) {
        GameCodeRegistry::getInstance().removeGameCode(gameCode);
        log_info("Game code " + gameCode + " removed from active games");
    }
    
    // Close socket and listener
    socket.disconnect();
    listener.close();
    
    role = NetworkRole::NONE;
    gameCode = "";
}

bool GameCodeRegistry::addGameCode(const std::string& code, const std::string& ip) {
        std::lock_guard<std::mutex> lock(codesMutex);
        if (activeGameCodes.find(code) != activeGameCodes.end()) {
            return false; // Code already exists
        }
        activeGameCodes[code] = ip;
        std::cout << "DEBUG: Added game code " << code << " with IP " << ip << std::endl;
        std::cout << "DEBUG: Active codes count: " << activeGameCodes.size() << std::endl;
        return true;
    }

std::string GameCodeRegistry::getIPForCode (const std::string& code) {
        std::lock_guard<std::mutex> lock(codesMutex);
        std::cout << "DEBUG: Looking for code " << code << std::endl;
        std::cout << "DEBUG: Active codes count: " << activeGameCodes.size() << std::endl;
        for (const auto& pair : activeGameCodes) {
            std::cout << "DEBUG: Available code: " << pair.first << " -> " << pair.second << std::endl;
        }
        
        auto it = activeGameCodes.find(code);
        if (it != activeGameCodes.end()) {
            return it->second;
        }
        return "";
    }

void GameCodeRegistry::removeGameCode(const std::string& code) {
        std::lock_guard<std::mutex> lock(codesMutex);
        activeGameCodes.erase(code);
        std::cout << "DEBUG: Removed game code " << code << std::endl;
    }

std::vector<std::string> GameCodeRegistry::getAllCodes() {
        std::lock_guard<std::mutex> lock(codesMutex);
        std::vector<std::string> codes;
        for (const auto& pair : activeGameCodes) {
            codes.push_back(pair.first);
        }
        return codes;
    }
#include "network.hpp"
#include "log.hpp"

#if RUN_NETWORK

NetworkManager::NetworkManager() : serverSocket(-1), clientSocket(-1), role(NetworkRole::NONE), isConnected(false), shouldStop(false) {}

NetworkManager::~NetworkManager() { cleanup(); }

std::string NetworkManager::getLocalIP() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) return "127.0.0.1"; // default
    
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8"); // Google public DNS
    serv.sin_port = htons(53);
    
    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
    if (err == -1) {
        close(sock);
        return "127.0.0.1";
    }
    
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*)&name, &namelen);
    
    close(sock);
    
    if (err == -1) return "127.0.0.1";
    
    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &name.sin_addr, buffer, INET_ADDRSTRLEN);
    
    return std::string(buffer);
}

bool NetworkManager::runHost(int port) {
    cleanup();
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        log_error("Error creating server socket");
        return false;
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Set socket timeout - shorter for faster cleanup
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;  // 500ms timeout
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        log_error("Error binding server socket");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    if (listen(serverSocket, 1) == -1) {
        log_error("Error listening on server socket");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    role = NetworkRole::HOST;
    log_info("Server started on " + getLocalIP());
    
    startListening();
    return true;
}

bool NetworkManager::runClient(const std::string& host_ip, int port) {
    cleanup();
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        log_error("Error creating client socket");
        return false;
    }
    
    // Set socket timeout - shorter for faster cleanup
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;  // 500ms timeout
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host_ip.c_str(), &serverAddr.sin_addr) <= 0) {
        log_warning("Invalid IP address");
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        log_error("Error connecting to server");
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    role = NetworkRole::CLIENT;
    isConnected = true;
    log_info("Connected to server " + host_ip + ":" + std::to_string(port));
    
    startListening();
    return true;
}

void NetworkManager::startListening() {
    shouldStop = false;
    listenerThread = std::thread(&NetworkManager::listenForMessages, this);
}

void NetworkManager::stopListening() {
    shouldStop = true;
    if (listenerThread.joinable()) listenerThread.join();
}

void NetworkManager::listenForMessages() {
    if (role == NetworkRole::HOST) {
        handleClientConnection();
        // Set timeout for client socket too
        if (clientSocket != -1) {
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 500000;  // 500ms timeout
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        }
    }
    
    int activeSocket = (role == NetworkRole::HOST) ? clientSocket : clientSocket;
    
    while (!shouldStop && activeSocket != -1) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytesReceived = recv(activeSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesReceived > 0) {
            std::string rawData(buffer, bytesReceived);
            NetworkMessage msg = parseMessage(rawData);
            
            std::lock_guard<std::mutex> lock(queueMutex);
            messageQueue.push(msg);
        } else if (bytesReceived == 0) {
            // Connection closed
            log_warning("Connection closed by peer");
            isConnected = false;
            break;
        } else {
            // Check if it's a timeout (expected behavior)
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            else {
                // Actual error occurred
                if (!shouldStop) {
                    log_error("Error receiving data");
                    isConnected = false;
                    break;
                }
            }
        }
    }
}

void NetworkManager::handleClientConnection() {
    if (role != NetworkRole::HOST) return;
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    if (clientSocket != -1) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        log_info("Client connected from " + std::string(clientIP));
        isConnected = true;
    }
}

void NetworkManager::sendMessage(const NetworkMessage& msg) {
    if (!isConnected) return;
    
    std::string serialized = serializeMessage(msg);
    int activeSocket = (role == NetworkRole::HOST) ? clientSocket : clientSocket;
    
    if (activeSocket != -1) send(activeSocket, serialized.c_str(), serialized.length(), 0);
}

bool NetworkManager::hasMessages() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return !messageQueue.empty();
}

NetworkMessage NetworkManager::getNextMessage() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (messageQueue.empty()) return {"", "", ""};
    
    NetworkMessage msg = messageQueue.front();
    messageQueue.pop();
    return msg;
}

NetworkMessage NetworkManager::parseMessage(const std::string& rawData) {
    NetworkMessage msg;
    
    // Simple parsing format: TYPE|SENDER|DATA
    size_t firstDelim = rawData.find('|');
    size_t secondDelim = rawData.find('|', firstDelim + 1);
    
    if (firstDelim != std::string::npos && secondDelim != std::string::npos) {
        msg.type = rawData.substr(0, firstDelim);
        msg.sender = rawData.substr(firstDelim + 1, secondDelim - firstDelim - 1);
        msg.data = rawData.substr(secondDelim + 1);
    } else {
        msg.type = "RAW";
        msg.sender = "UNKNOWN";
        msg.data = rawData;
    }
    return msg;
}

std::string NetworkManager::serializeMessage(const NetworkMessage& msg) {
    return msg.type + "|" + msg.sender + "|" + msg.data;
}

void NetworkManager::cleanup() {
    // Signal threads to stop first
    shouldStop = true;
    isConnected = false;
    
    // Close sockets immediately to interrupt blocking calls
    if (serverSocket != -1) {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }
    if (clientSocket != -1) {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }
    
    // Now join the thread (should exit quickly due to socket closure)
    if (listenerThread.joinable()) {
        // Use a shorter timeout since sockets are already closed
        bool threadExited = false;
        
        // Check if thread exits within 100ms
        auto future = std::async(std::launch::async, [this, &threadExited]() {
            listenerThread.join();
            threadExited = true;
        });
        
        if (future.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout) {
            log_info("Network thread cleanup timeout, detaching thread");
            listenerThread.detach();
        }
    }
    
    // Clear message queue
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!messageQueue.empty()) messageQueue.pop();
    
    role = NetworkRole::NONE;
}

#endif
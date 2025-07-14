// #include "network.hpp"

// std::string NetworkManager::getLocalIP() {
//     char buffer[INET_ADDRSTRLEN];
//     struct sockaddr_in temp;
//     temp.sin_family = AF_INET;
//     temp.sin_port = htons(80);
//     inet_pton(AF_INET, "8.8.8.8", &temp.sin_addr); // dummy external IP

//     int sock = socket(AF_INET, SOCK_DGRAM, 0);
//     connect(sock, (struct sockaddr*)&temp, sizeof(temp));

//     struct sockaddr_in name;
//     socklen_t namelen = sizeof(name);
//     getsockname(sock, (struct sockaddr*)&name, &namelen);
//     inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
//     close(sock);

//     return std::string(buffer);
// }
// void NetworkManager::runHost(int port) {
//     //port = 8080; // Default port, can be changed as needed

//     //Show the local IP address to the screen
//     std::cout << "Your IP address: " << getLocalIP() << std::endl;

//     //Connecting to the client
//     int server_fd, client_socket;
//     struct sockaddr_in address;
//     // char buffer[1024] = {0};
//     int addrlen = sizeof(address);

//     server_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_fd == 0) {
//         perror("socket failed");
//         return;
//     }

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY; // accept from any IP
//     address.sin_port = htons(port);

//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
//         perror("bind failed");
//         return;
//     }

//     if (listen(server_fd, 1) < 0) {
//         perror("listen failed");
//         return;
//     }

//     std::cout << "Waiting for a connection on port " << port << "...\n";
//     client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
//     if (client_socket < 0) {
//         perror("accept failed");
//         return;
//     }

//     std::cout << "Connected to client: " << inet_ntoa(address.sin_addr) << "\n";

//     // close(sock);
// }
// void NetworkManager::runClient(const std::string& host_ip, int port) {
//     int sock = 0;
//     struct sockaddr_in serv_addr;
//     // char buffer[1024] = {0};

//     sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0) {
//         perror("socket creation error");
//         return;
//     }

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(port);

//     if (inet_pton(AF_INET, host_ip.c_str(), &serv_addr.sin_addr) <= 0) {
//         std::cerr << "Invalid address/Address not supported\n";
//         return;
//     }

//     std::cout << "Connecting to " << host_ip << "...\n";
//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
//         perror("connection failed");
//         return;
//     }

//     std::cout << "Connected to host.\n";

//     // close(sock);
// }
// void NetworkManager::receiveData(int sock, char buffer[]) {
//     memset(buffer, 0, sizeof(buffer));
//     read(sock, buffer, 1024);
// }
// void NetworkManager::sendData(int sock, std::string& message) {
//     send(sock, message.c_str(), message.length(), 0);
// }

#include "network.hpp"

#if RUN_NETWORK

NetworkManager::NetworkManager() : serverSocket(-1), clientSocket(-1), role(NetworkRole::NONE), 
                  isConnected(false), shouldStop(false) {}

NetworkManager::~NetworkManager() {
    cleanup();
}

std::string NetworkManager::getLocalIP() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) return "127.0.0.1";
    
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
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
        std::cerr << "Error creating server socket" << std::endl;
        return false;
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket" << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    if (listen(serverSocket, 1) == -1) {
        std::cerr << "Error listening on server socket" << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return false;
    }
    
    role = NetworkRole::HOST;
    std::cout << "Server started on " << getLocalIP() << ":" << port << std::endl;
    std::cout << "Waiting for client connection..." << std::endl;
    
    startListening();
    return true;
}

bool NetworkManager::runClient(const std::string& host_ip, int port) {
    cleanup();
    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating client socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host_ip.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address" << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    role = NetworkRole::CLIENT;
    isConnected = true;
    std::cout << "Connected to server " << host_ip << ":" << port << std::endl;
    
    startListening();
    return true;
}

void NetworkManager::startListening() {
    shouldStop = false;
    listenerThread = std::thread(&NetworkManager::listenForMessages, this);
}

void NetworkManager::stopListening() {
    shouldStop = true;
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
}

void NetworkManager::listenForMessages() {
    if (role == NetworkRole::HOST) {
        handleClientConnection();
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
            std::cout << "Connection closed by peer" << std::endl;
            isConnected = false;
            break;
        } else {
            // Error occurred
            if (!shouldStop) {
                std::cerr << "Error receiving data" << std::endl;
                isConnected = false;
                break;
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
        std::cout << "Client connected from " << clientIP << std::endl;
        isConnected = true;
    }
}

void NetworkManager::sendMessage(const NetworkMessage& msg) {
    if (!isConnected) return;
    
    std::string serialized = serializeMessage(msg);
    int activeSocket = (role == NetworkRole::HOST) ? clientSocket : clientSocket;
    
    if (activeSocket != -1) {
        send(activeSocket, serialized.c_str(), serialized.length(), 0);
    }
}

bool NetworkManager::hasMessages() {
    std::lock_guard<std::mutex> lock(queueMutex);
    return !messageQueue.empty();
}

NetworkMessage NetworkManager::getNextMessage() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (messageQueue.empty()) {
        return {"", "", ""};
    }
    
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
    shouldStop = true;
    isConnected = false;
    
    if (listenerThread.joinable()) {
        listenerThread.join();
    }
    
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    if (clientSocket != -1) {
        close(clientSocket);
        clientSocket = -1;
    }
    
    role = NetworkRole::NONE;
}

#endif
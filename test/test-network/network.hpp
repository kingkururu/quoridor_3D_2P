// network.hpp

#pragma once

#define RUN_NETWORK 1
#define BUFFER_SIZE 1024

#if RUN_NETWORK

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <future>

enum class NetworkRole {
    NONE,
    HOST,
    CLIENT
};

struct NetworkMessage {
    std::string type;
    std::string data;
    std::string sender;
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Core networking functions
    std::string getLocalIP();
    bool runHost(int port);
    bool runClient(const std::string& host_ip, int port);
    void cleanup();
    
    // Message handling
    void sendMessage(const NetworkMessage& msg);
    bool hasMessages();
    NetworkMessage getNextMessage();
    
    // Status functions
    bool isNetworkConnected() const { return isConnected; }
    NetworkRole getRole() const { return role; }
    void startListening();
    void stopListening();
    
private:
    int serverSocket;
    int clientSocket;
    NetworkRole role;
    std::atomic<bool> isConnected;
    std::atomic<bool> shouldStop;
    
    // Threading for async communication
    std::thread listenerThread;
    std::queue<NetworkMessage> messageQueue;
    std::mutex queueMutex;
    
    // Internal functions
    void listenForMessages();
    void handleClientConnection();
    NetworkMessage parseMessage(const std::string& rawData);
    std::string serializeMessage(const NetworkMessage& msg);
};

#endif
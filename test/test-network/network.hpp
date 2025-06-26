#pragma once

#define RUN_NETWORK 1 // Set to 1 to enable

#if RUN_NETWORK 
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
class NetworkManager {
    public:
    std::string getLocalIP();
    void runHost(int port);
    void runClient(const std::string& host_ip, int port);
    void receiveData(int sock, char buffer[]);
    void sendData(int sock, std::string& message);
}
#else 

#endif 


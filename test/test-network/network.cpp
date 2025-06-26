#include "network.hpp"

std::string NetworkManager::getLocalIP() {
    char buffer[INET_ADDRSTRLEN];
    struct sockaddr_in temp;
    temp.sin_family = AF_INET;
    temp.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &temp.sin_addr); // dummy external IP

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    connect(sock, (struct sockaddr*)&temp, sizeof(temp));

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    getsockname(sock, (struct sockaddr*)&name, &namelen);
    inet_ntop(AF_INET, &name.sin_addr, buffer, sizeof(buffer));
    close(sock);

    return std::string(buffer);
}
void NetworkManager::runHost(int port) {
    //port = 8080; // Default port, can be changed as needed

    //Show the local IP address to the screen
    std::cout << "Your IP address: " << getLocalIP() << std::endl;

    //Connecting to the client
    int server_fd, client_socket;
    struct sockaddr_in address;
    // char buffer[1024] = {0};
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // accept from any IP
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        return;
    }

    std::cout << "Waiting for a connection on port " << port << "...\n";
    client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        perror("accept failed");
        return;
    }

    std::cout << "Connected to client: " << inet_ntoa(address.sin_addr) << "\n";

    // close(sock);
}
void NetworkManager::runClient(const std::string& host_ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    // char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        return;
    }

    std::cout << "Connecting to " << host_ip << "...\n";
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        return;
    }

    std::cout << "Connected to host.\n";

    // close(sock);
}
void NetworkManager::receiveData(int sock, char buffer[]) {
    memset(buffer, 0, sizeof(buffer));
    read(sock, buffer, 1024);
}
void NetworkManager::sendData(int sock, std::string& message) {
    send(sock, message.c_str(), message.length(), 0);
}
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const int MAX_BUFFER_SIZE = 4096;
const std::string ACCOUNT_FILE = "account.txt";

std::mutex mutex;

struct ClientInfo {
    SOCKET socket;
    std::string username;
};

std::vector<ClientInfo> clients;

void HandleClient(ClientInfo client);
bool AuthenticateUser(const std::string& username);
bool IsUserLoggedIn(const std::string& username);
void AddUserToLoggedInList(const std::string& username);
void RemoveUserFromLoggedInList(const std::string& username);
bool PostArticle(const std::string& username, const std::string& article);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: Server.exe PortNumber" << std::endl;
        return 0;
    }

    int portNumber = std::stoi(argv[1]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. Listening on port " << portNumber << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting client connection" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        ClientInfo client;
        client.socket = clientSocket;

        std::thread clientThread(HandleClient, client);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

void HandleClient(ClientInfo client) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    int bytesRead = recv(client.socket, buffer, MAX_BUFFER_SIZE - 1, 0);
    if (bytesRead <= 0) {
        closesocket(client.socket);
        return;
    }

    std::string message(buffer);

    if (message.find("USER") == 0) {
        std::string username = message.substr(5);
        if (AuthenticateUser(username)) {
            if (IsUserLoggedIn(username)) {
                send(client.socket, "13", 2, 0); // User already logged in
            }
            else {
                AddUserToLoggedInList(username);
                client.username = username;
                clients.push_back(client);
                send(client.socket, "10", 2, 0); // Authentication successful
            }
        }
        else {
            send(client.socket, "12", 2, 0); // Invalid username
        }
    }
    else if (message.find("POST") == 0) {
        if (client.username.empty()) {
            send(client.socket, "21", 2, 0); // Not logged in
        }
        else {
            std::string article = message.substr(5);
            if (PostArticle(client.username, article)) {
                send(client.socket, "20", 2, 0); // Post successful
            }
            else {
                send(client.socket, "99", 2, 0); // Unknown request type
            }
        }
    }
    else if (message.find("BYE") == 0) {
        if (client.username.empty()) {
            send(client.socket, "21", 2, 0); // Not logged in
        }
        else {
            RemoveUserFromLoggedInList(client.username);
            send(client.socket, "30", 2, 0); // Logout successful
            closesocket(client.socket);
        }
    }
    else {
        send(client.socket, "99", 2, 0); // Unknown request type
    }

    closesocket(client.socket);
}

bool AuthenticateUser(const std::string& username) {
    std::ifstream file(ACCOUNT_FILE);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string savedUsername, status;
        std::istringstream iss(line);
        if (iss >> savedUsername >> status) {
            if (savedUsername == username && status == "0") {
                return true;
            }
        }
    }

    return false;
}

bool IsUserLoggedIn(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& client : clients) {
        if (client.username == username) {
            return true;
        }
    }
    return false;
}

void AddUserToLoggedInList(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& client : clients) {
        if (client.username.empty()) {
            client.username = username;
            break;
        }
    }
}

void RemoveUserFromLoggedInList(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& client : clients) {
        if (client.username == username) {
            client.username = "";
            break;
        }
    }
}

bool PostArticle(const std::string& username, const std::string& article) {
    std::ofstream file(ACCOUNT_FILE, std::ios::app);
    if (!file) {
        return false;
    }

    file << username << ": " << article << std::endl;
    return true;
}

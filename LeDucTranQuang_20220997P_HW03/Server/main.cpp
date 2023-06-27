#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

const int MAX_BUFFER_SIZE = 4096;

std::mutex mutex;
std::vector<SOCKET> clientSockets;

void handleClient(SOCKET clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::string request(buffer);

            // Parse the request
            std::string command;
            std::string content;
            size_t pos = request.find(' ');
            if (pos != std::string::npos) {
                command = request.substr(0, pos);
                content = request.substr(pos + 1);
            }
            else {
                command = request;
            }

            // Handle the request
            std::string response;
            if (command == "USER") {
                // Check if the account exists and is active
                std::ifstream accountFile("account.txt");
                std::string line;
                bool accountFound = false;
                bool accountActive = false;
                while (std::getline(accountFile, line)) {
                    size_t pos = line.find(' ');
                    if (pos != std::string::npos) {
                        std::string username = line.substr(0, pos);
                        std::string status = line.substr(pos + 1);
                        if (username == content) {
                            accountFound = true;
                            if (status == "0") {
                                accountActive = true;
                            }
                            break;
                        }
                    }
                }
                accountFile.close();

                if (!accountFound) {
                    response = "12"; // Account does not exist
                }
                else if (!accountActive) {
                    response = "11"; // Account is locked
                }
                else {
                    // Check if the account is already logged in on another client
                    bool alreadyLoggedIn = false;
                    mutex.lock();
                    for (SOCKET socket : clientSockets) {
                        if (socket != clientSocket) {
                            response = "13"; // Account is already logged in
                            alreadyLoggedIn = true;
                            break;
                        }
                    }
                    if (!alreadyLoggedIn) {
                        clientSockets.push_back(clientSocket);
                        response = "10"; // Successful login
                    }
                    mutex.unlock();
                }
            }
            else if (command == "POST") {
                // Check if the client is logged in
                bool loggedIn = false;
                mutex.lock();
                for (SOCKET socket : clientSockets) {
                    if (socket == clientSocket) {
                        loggedIn = true;
                        break;
                    }
                }
                mutex.unlock();

                if (loggedIn) {
                    // Process the article content (e.g., store in a file)
                    std::ofstream articleFile("articles.txt", std::ios::app);
                    articleFile << content << std::endl;
                    articleFile.close();
                    response = "20"; // Successful article posting
                }
                else {
                    response = "21"; // Not logged in
                }
            }
            else if (command == "BYE") {
                // Remove the client socket from the list
                mutex.lock();
                auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
                if (it != clientSockets.end()) {
                    clientSockets.erase(it);
                }
                mutex.unlock();

                response = "30"; // Successful logout
            }
            else {
                response = "99"; // Invalid command
            }

            // Send the response back to the client
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else if (bytesReceived == 0) {
            // Client disconnected
            break;
        }
        else {
            // Error occurred
            std::cerr << "Error in recv(). Quitting..." << std::endl;
            break;
        }

        memset(buffer, 0, MAX_BUFFER_SIZE);
    }

    closesocket(clientSocket);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PortNumber" << std::endl;
        return 1;
    }

    int portNumber = std::stoi(argv[1]);

    // Initialize Winsock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    // Create a socket
    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(listeningSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket to IP and port" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for connections" << std::endl;
        return 1;
    }

    std::cout << "Server started. Waiting for connections..." << std::endl;

    // Accept and handle client connections
    while (true) {
        // Accept a new client connection
        sockaddr_in clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddress, &clientAddressSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection" << std::endl;
            break;
        }

        // Create a thread to handle the client
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    // Cleanup Winsock
    closesocket(listeningSocket);
    WSACleanup();

    return 0;
}

#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const int MAX_BUFFER_SIZE = 4096;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: Client.exe ServerIP ServerPort" << std::endl;
        return 0;
    }

    std::string serverIP = argv[1];
    int serverPort = std::stoi(argv[2]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    while (true) {
        std::string command;
        std::cout << "Enter a command (USER, POST, BYE): ";
        std::cin >> command;

        if (command == "USER") {
            std::string username;
            std::cout << "Enter username: ";
            std::cin >> username;

            std::string request = "USER " + username;
            send(clientSocket, request.c_str(), request.size(), 0);

            int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string response(buffer);

                if (response == "10") {
                    std::cout << "Authentication successful" << std::endl;
                }
                else if (response == "11") {
                    std::cout << "Account is locked" << std::endl;
                }
                else if (response == "12") {
                    std::cout << "Invalid username" << std::endl;
                }
                else if (response == "13") {
                    std::cout << "User already logged in from another client" << std::endl;
                }
                else {
                    std::cout << "Unknown response from server" << std::endl;
                }
            }
            else {
                std::cerr << "Error receiving response from server" << std::endl;
                break;
            }
        }
        else if (command == "POST") {
            std::string article;
            std::cout << "Enter article content: ";
            std::cin.ignore();
            std::getline(std::cin, article);

            std::string request = "POST " + article;
            send(clientSocket, request.c_str(), request.size(), 0);

            int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string response(buffer);

                if (response == "20") {
                    std::cout << "Article posted successfully" << std::endl;
                }
                else if (response == "21") {
                    std::cout << "You need to login first" << std::endl;
                }
                else {
                    std::cout << "Unknown response from server" << std::endl;
                }
            }
            else {
                std::cerr << "Error receiving response from server" << std::endl;
                break;
            }
        }
        else if (command == "BYE") {
            std::string request = "BYE";
            send(clientSocket, request.c_str(), request.size(), 0);

            int bytesReceived = recv(clientSocket, buffer, MAX_BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string response(buffer);

                if (response == "30") {
                    std::cout << "Logout successful" << std::endl;
                }
                else if (response == "21") {
                    std::cout << "You need to login first" << std::endl;
                }
                else {
                    std::cout << "Unknown response from server" << std::endl;
                }
            }
            else {
                std::cerr << "Error receiving response from server" << std::endl;
                break;
            }

            break; // Exit the loop and terminate the client
        }
        else {
            std::cout << "Invalid command" << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

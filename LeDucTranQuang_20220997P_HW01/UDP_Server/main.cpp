#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 1024;

void handleError(const std::string& errorMessage) {
	std::cerr << errorMessage << std::endl;
	exit(1);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		handleError("Sử dụng: UDP_Client.exe ServerIPAddress ServerPortNumber");
	}

	// Khởi tạo Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		handleError("Không thể khởi tạo Winsock.");
	}

	SOCKET clientSocket;
	struct sockaddr_in serverAddress;
	char buffer[BUFFER_SIZE];

	// Tạo socket
	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		handleError("Không thể tạo socket.");
	}

	// Chuẩn bị địa chỉ server
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(argv[2]));

	if ((serverAddress.sin_addr.s_addr = inet_addr(argv[1])) == INADDR_NONE) {
		handleError("Địa chỉ IP server không hợp lệ.");
	}

	while (true) {
		std::string domain;

		// Nhập tên miền từ người dùng
		std::cout << "Nhập tên miền (hoặc để trống để thoát): ";
		std::getline(std::cin, domain);

		if (domain.empty()) {
			break;
		}

		// Gửi yêu cầu tên miền tới server
		if (sendto(clientSocket, domain.c_str(), domain.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
			handleError("Không thể gửi dữ liệu tới server.");
		}

		// Nhận phản hồi từ server
		struct sockaddr_in responseAddress;
		int responseAddressSize = sizeof(responseAddress);
		memset(&responseAddress, 0, sizeof(responseAddress));

		ssize_t receivedBytes = recvfrom(clientSocket, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&responseAddress, &responseAddressSize);
		if (receivedBytes == SOCKET_ERROR) {
			handleError("Không thể nhận dữ liệu từ server.");
		}

		buffer[receivedBytes] = '\0';
		std::cout << "Địa chỉ IP phân giải: " << buffer << std::endl;
	}

	// Đóng socket
	closesocket(clientSocket);

	// Gỡ bỏ Winsock
	WSACleanup();

	return 0;
}

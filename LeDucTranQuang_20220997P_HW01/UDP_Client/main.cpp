#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

void handleError(const std::string& errorMessage) {
	std::cerr << errorMessage << std::endl;
	exit(1);
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		handleError("Sử dụng: UDP_Client.exe ServerIPAddress ServerPortNumber");
	}

	int clientSocket;
	struct sockaddr_in serverAddress;
	char buffer[BUFFER_SIZE];

	// Tạo socket
	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		handleError("Không thể tạo socket.");
	}

	// Chuẩn bị địa chỉ server
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(atoi(argv[2]));

	if (inet_pton(AF_INET, argv[1], &(serverAddress.sin_addr)) <= 0) {
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
		ssize_t sentBytes = sendto(clientSocket, domain.c_str(), domain.length(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
		if (sentBytes == -1) {
			handleError("Không thể gửi dữ liệu tới server.");
		}

		// Nhận phản hồi từ server
		ssize_t receivedBytes = recvfrom(clientSocket, buffer, BUFFER_SIZE - 1, 0, nullptr, nullptr);
		if (receivedBytes == -1) {
			handleError("Không thể nhận dữ liệu từ server.");
		}

		buffer[receivedBytes] = '\0';
		std::cout << "Địa chỉ IP phân giải: " << buffer << std::endl;
	}

	// Đóng socket
	close(clientSocket);

	return 0;
}

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "Cú pháp: TCP_Client.exe ServerIPAddress ServerPortNumber" << std::endl;
		return 1;
	}

	int clientSocket;
	std::string serverIPAddress = argv[1];
	int serverPortNumber = std::atoi(argv[2]);

	struct sockaddr_in serverAddress;

	// Tạo socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == -1) {
		std::cerr << "Không thể tạo socket." << std::endl;
		return 1;
	}

	// Thiết lập địa chỉ server
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(serverIPAddress.c_str());
	serverAddress.sin_port = htons(serverPortNumber);

	// Kết nối tới server
	if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "Không thể kết nối tới server." << std::endl;
		close(clientSocket);
		return 1;
	}

	std::cout << "Đã kết nối tới server " << serverIPAddress << " trên cổng " << serverPortNumber << std::endl;

	std::string userInput;

	do {
		std::cout << "Nhập một xâu (hoặc nhập xâu rỗng để kết thúc): ";
		std::getline(std::cin, userInput);

		// Gửi xâu tới server
		if (send(clientSocket, userInput.c_str(), userInput.length(), 0) == -1) {
			std::cerr << "Lỗi trong quá trình gửi dữ liệu." << std::endl;
			close(clientSocket);
			return 1;
		}

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));

		// Nhận kết quả từ server
		if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1) {
			std::cerr << "Lỗi trong quá trình nhận dữ liệu." << std::endl;
			close(clientSocket);
			return 1;
		}

		std::string result(buffer);
		std::cout << "Kết quả từ server: " << result << std::endl;

	} while (!userInput.empty());

	close(clientSocket);

	return 0;
}

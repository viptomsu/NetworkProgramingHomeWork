#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Cú pháp: TCP_Server.exe PortNumber" << std::endl;
		return 1;
	}

	int serverSocket;
	int clientSocket;
	int portNumber = std::atoi(argv[1]);

	struct sockaddr_in serverAddress, clientAddress;
	socklen_t clientAddressLength = sizeof(clientAddress);

	// Tạo socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		std::cerr << "Không thể tạo socket." << std::endl;
		return 1;
	}

	// Thiết lập địa chỉ server
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(portNumber);

	// Gắn socket với địa chỉ
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
		std::cerr << "Không thể gắn socket với địa chỉ." << std::endl;
		close(serverSocket);
		return 1;
	}

	// Lắng nghe kết nối từ client
	if (listen(serverSocket, 1) == -1) {
		std::cerr << "Lỗi trong quá trình lắng nghe." << std::endl;
		close(serverSocket);
		return 1;
	}

	std::cout << "Server đang lắng nghe kết nối tại cổng " << portNumber << std::endl;

	while (true) {
		// Chấp nhận kết nối từ client
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
		if (clientSocket == -1) {
			std::cerr << "Lỗi trong quá trình chấp nhận kết nối." << std::endl;
			close(serverSocket);
			return 1;
		}

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, sizeof(buffer));

		// Nhận xâu từ client
		if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1) {
			std::cerr << "Lỗi trong quá trình nhận dữ liệu." << std::endl;
			close(clientSocket);
			continue;
		}

		std::string receivedString(buffer);

		// Kiểm tra xâu chứa ký tự khác chữ số
		bool error = false;
		for (char c : receivedString) {
			if (!isdigit(c)) {
				error = true;
				break;
			}
		}

		// Tính tổng các chữ số trong xâu
		int sum = 0;
		if (!error) {
			for (char c : receivedString) {
				sum += (c - '0');
			}
		}

		// Gửi kết quả cho client
		std::string result = error ? "Lỗi: Xâu không chứa chỉ chữ số." : std::to_string(sum);
		if (send(clientSocket, result.c_str(), result.length(), 0) == -1) {
			std::cerr << "Lỗi trong quá trình gửi dữ liệu." << std::endl;
		}

		close(clientSocket);
	}

	close(serverSocket);

	return 0;
}

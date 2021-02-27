#define _WIN32_WINNT 0x501

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>

// Linker libary
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define PORT "24500"

int main() {
	WSADATA wsaData;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	SOCKET ConnectSocket = INVALID_SOCKET;

	int iResult;

	//WSA Start up
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSA Start up failed.\n";
		return 1;
	}

	//winsock
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//getaddrinfo
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "Unable get address information." << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	//Connect to an address until success
	std::cout << "Connecting to server...\n";
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cout << "Socket failed to create." << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			if (ptr->ai_next == NULL) {
				std::cout << "Connect failed. " << WSAGetLastError() << std::endl;
				WSACleanup();
				return 1;
			}
			continue;
		}
		else {
			std::cout << "Server Connected.\n";
		}
		break;
	}

	freeaddrinfo(result);

	static bool c = false;
	if(iResult == 0) std::cout << "Welcome to Simple TCP Chat.\n";

	while (!c) {
		//send buffer to server
		std::cout << "Send: \n";

		char sendBuf[512];
		std::cin.getline(sendBuf, sizeof(sendBuf));
		if (send(ConnectSocket, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
			std::cout << "Failed to send message. " << WSAGetLastError() << std::endl;
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		else if (strlen(sendBuf) == 0) {
			c = true;
		}

		memset(sendBuf, 0, sizeof(sendBuf)); // clear sendBuf

		//recv buffer from server
		char recvBuf[512];
		if (recv(ConnectSocket, recvBuf, sizeof(recvBuf), 0) == SOCKET_ERROR) {
			std::cout << "Failed to receive message. " << WSAGetLastError() << std::endl;
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		else if (strlen(recvBuf) == 0) {
			c = true;
		}
		else {
			std::cout << "Server: " << recvBuf << std::endl;
		}
	}

	if(c) std::cout << "Client chat closing...\n";

	//shutdown
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
} 
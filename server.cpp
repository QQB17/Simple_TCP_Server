#define _WIN32_WINNT 0x501

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>

// Linker libary
#pragma comment(lib, "lws2_32.lib");

#define PORT "24500"

int main() {
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
	struct addrinfo* result, hints;

	int iResult;

	// WSA Start up
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSA Start Up failed." << iResult << std::endl;
		return 1;
	}

	//set hints struct
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "Address information can't get." << std::endl;
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		std::cout << "Listen Socket created failed." << std::endl;
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Unable to bind." << std::endl;
		return 1;
	}

	freeaddrinfo(result);

	//listen
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult != 0) {
		std::cout << "Listen socket failed: " << WSAGetLastError() << std::endl;
		return 1;
	}
	else {
		std::cout << "Waiting for client connect...\n";
	}

	//accept
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
		return 1;
	} else {
		std::cout << "Client Connected.\n";
	}
 
	closesocket(ListenSocket);

	static bool s = false;
	std::cout << "Welcome to Simple TCP Chat.\n";

	while(!s){
		char recvBuf[512];

		if (recv(ClientSocket, recvBuf, sizeof(recvBuf), 0) == SOCKET_ERROR) {
			std::cout << "Failed to receive message. " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		else if (strlen(recvBuf) == 0) {
			s = true;
		}

		std::cout << "Client: " << recvBuf << std::endl;

		std::cout << "Send: \n";
		char sendBuf[512];
		std::cin.getline(sendBuf, sizeof(sendBuf));	

		if (send(ClientSocket, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
			std::cout << "Failed to send message. " << WSAGetLastError() << std::endl;
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		else if (strlen(sendBuf) == 0) {
			s = true;
		}
		memset(sendBuf, 0, sizeof(sendBuf));
	}

	if (s) std::cout << "Server chat closing...\n";

	//shutdown
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult != 0) {
		std::cout << "Shutdown failed. " << WSAGetLastError() << std::endl;
		return 1;
	}

	//clear
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
} 
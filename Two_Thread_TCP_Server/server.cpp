#define _WIN32_WINNT 0x501

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>

// Linker libary
#pragma comment(lib, "lws2_32")

#define PORT "24000"
#define errorLog(x) std::cout << x << " failed with " << WSAGetLastError() << std::endl

bool s_Finished = false;

// Function to receive buffer
DWORD WINAPI serverReceive(LPVOID lpParam) {
	char recvBuf[512];
	SOCKET client = *(SOCKET*)lpParam;

	while (!s_Finished) {
		if (recv(client, recvBuf, sizeof(recvBuf), 0) == SOCKET_ERROR) {
			errorLog("Recv function");
			return -1;
		}else if (strcmp(recvBuf, "exit") == 0) {
			s_Finished = true;
		}
		else {
			std::cout << "Client: " << recvBuf << std::endl;
			memset(recvBuf, 0, sizeof(recvBuf));
		}
	}
	std::cout << "Client disconnect...\n";
	
	return 0;
}

// Function to send buffer
DWORD WINAPI serverSend(LPVOID lpParam) {
	char sendBuf[512];
	SOCKET Client = *(SOCKET*)lpParam;

	while (!s_Finished) {
		std::cin.getline(sendBuf, sizeof(sendBuf));

		if (send(Client, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
			errorLog("Send function");
			return -1;
		}
		else if (strcmp(sendBuf, "exit") == 0) {
			s_Finished = true;
		}
	};
	std::cout << "Server disconnect...\n";

	return 0;
}

int main() {
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, hints;

	int iResult;
	
	// WSA Start up
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		errorLog("WSA Start up");
		return 1;
	}

	// Set address information
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//getaddrinfo
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		errorLog("Get address information");
		return 1;
	}

	//creak socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		errorLog("Listen socket creation");
		WSACleanup();
		return 1;
	}

	//bind address 
	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		errorLog("Bind address");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	//listen
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult != 0) {
		errorLog("Listen");
		WSACleanup();
		return 1;
	}
	else {
		std::cout << "Waiting for client connect...\n";
	}

	//accept client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		errorLog("Client socket created");
		WSACleanup();
		return 1;
	}
	else {
		// two way communication
		std::cout << "Client Connect!" << std::endl;
		std::cout << "Live Chat Started." << std::endl;

		DWORD tid;

		HANDLE t1 = CreateThread(NULL, 0, serverReceive, &ClientSocket, 0, &tid); // Receive thread
		HANDLE t2 = CreateThread(NULL, 0, serverSend, &ClientSocket, 0, &tid); // Send thread

		if (t1 == 0 || t2 == 0) {
			errorLog("Thread creation");
			return 1;
		}
		else {
			std::cout << "Welcome to Live Chat.\n" << "Enter \"exit\" to disconnect.\n";;
		}

		WaitForSingleObject(t1, INFINITE);
		WaitForSingleObject(t2, INFINITE);

		std::cout << "Chat Closing...\n";
	}

	//clear
	closesocket(ClientSocket);
	closesocket(ListenSocket);
	WSACleanup();

	return 0;
}
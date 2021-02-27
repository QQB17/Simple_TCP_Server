#define _WIN32_WINNT 0x501

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>

// Linker libary
#pragma comment(lib, "lws2_32")

#define PORT "24000"
#define errorLog(x) std::cout << x << " failed with " << WSAGetLastError() << std::endl
#define successLog(x) std::cout << x << " success.\n"
#define ASSERT(x) if(x) return 1
#define LogCall(x,y) ASSERT(validCheck(x,y))

static bool validCheck(const int function, const char* message) {
	if (function != 0) {
		errorLog(message);
		WSACleanup();
		return 1;
	}
	else {
		successLog(message);
		return 0;
	}
}

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

	// WSA Start up
	LogCall(WSAStartup(MAKEWORD(2, 2), &wsaData), "WSA Start Up");

	// Set address information
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//getaddrinfo
	LogCall(getaddrinfo(NULL, PORT, &hints, &result),"Get address information");

	//creak socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == SOCKET_ERROR) {
		errorLog("Create Listen Socket");
		WSACleanup();
		return 1;
	}
	else {
		successLog("Create Listen Socket");
	}

	//bind address 
	LogCall(bind(ListenSocket, result->ai_addr, result->ai_addrlen), "Bind address");
	
	freeaddrinfo(result);

	//listen
	LogCall(listen(ListenSocket, SOMAXCONN), "Listening");
	std::cout << "Waiting for client connect...\n";

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
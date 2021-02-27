#define _WIN32_WINNT 0x501

#include <iostream>
#include <WS2tcpip.h>
#include <Windows.h>

// Linker libary
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define PORT "24000"
#define errorLog(x) std::cout << x << " failed with " << WSAGetLastError() << std::endl;
#define successLog(x) std::cout << x << " success.\n"
#define ASSERT(x) if(x) return 1;
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

bool c_Finished = false;

// Function to send buffer
DWORD WINAPI clientSend(LPVOID lpParam) {
	char sendBuf[512];
	SOCKET server = *(SOCKET*)lpParam;

	while (!c_Finished) {
		std::cin.getline(sendBuf, sizeof(sendBuf));

		if (send(server, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
			errorLog("Send message to server");
			return -1;
		}else if (strcmp(sendBuf, "exit") ==0) {
			c_Finished = true;
		}
	}
	std::cout << "Client disconnect...\n";
	return 0;
}

// Function to receive buffer
DWORD WINAPI clientRecv(LPVOID lpParam) {
	char recvBuf[512];
	SOCKET server = *(SOCKET*)lpParam;

	while (!c_Finished) {
		if (recv(server, recvBuf, sizeof(recvBuf), 0) == SOCKET_ERROR) {
			errorLog("Receive message from server");
			return -1;
		} else if (strcmp(recvBuf, "exit") ==0) {
			c_Finished = true;
		}
		else {
			std::cout << "Server: " << recvBuf << std::endl;
			memset(recvBuf, 0, sizeof(recvBuf));
		}
	}
	std::cout << "Server disconnect...\n";

	return 0;
}


int main(int argc, char** argv) {
	WSADATA wsaData;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo* ptr = NULL, * result = NULL, hints;

	int iResult;

	// WSA tart up
	LogCall(WSAStartup(MAKEWORD(2, 2), &wsaData), "WSA Start up");

	//Set address information
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Get address information
	LogCall(getaddrinfo("localhost", PORT, &hints, &result), "Get Address Information");
	
	// Try to create a socket and connect to the IP address
	std::cout << "Connecting to server...\n";
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		// create socket
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			errorLog("Client Socket creation");
			WSACleanup();
			return 1;
		}

		// connect to server
		iResult = connect(ClientSocket, ptr->ai_addr, ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
			// if there is nothing more, exit program
			if (ptr->ai_next == NULL) {
				errorLog("Server connected");
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

	// two way communication
	DWORD tid;

	HANDLE t1 = CreateThread(NULL, 0, clientSend, &ClientSocket, 0, &tid);
	HANDLE t2 = CreateThread(NULL, 0, clientRecv, &ClientSocket, 0, &tid);

	if (t1 == NULL || t2 == NULL) {
		std::cout << "Thread creation failed.\n";
		return 1;
	}
	else {
		std::cout << "Welcome to Live Chat.\n" << "Enter \"exit\" to disconnect.\n" ;
	}
	
	WaitForSingleObject(t1, INFINITE);
	WaitForSingleObject(t2, INFINITE);

	std::cout << "Chat Closing...\n";

	// clear
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}
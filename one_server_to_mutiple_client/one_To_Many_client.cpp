#define _WIN32_WINNT 0x501

#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"lws2_32")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define PORT "24000"

#define errorCall(x) std::cout << x << " failed with " << WSAGetLastError() << std::endl
#define successCall(x) std::cout << x << " success.\n"
#define ASSERT(x) if (x) return 1;
#define LogCall(x,y) ASSERT(validCheck(x,y))

static bool validCheck(const int function, const char* message) {
	if (function != 0) {
		errorCall(message);
		WSACleanup();
		return 1;
	}
	else {
		successCall(message);
		return 0;
	}
	return 1;
}


DWORD WINAPI sendMessage(LPVOID lpParam) {
	char sendBuf[512];

	SOCKET sock = *(SOCKET*)lpParam;
	while (true) {
		std::cin.getline(sendBuf, sizeof(sendBuf));
		if (strcmp(sendBuf, "exit") == 0) {
			break;
			return 0;
		}

		if (send(sock, sendBuf, sizeof(sendBuf), 0) == SOCKET_ERROR) {
			errorCall("Send message");
			return -1;
		}
		
	}
	return 0;
}

DWORD WINAPI recvMessage(LPVOID lpParam) {
	char recvBuf[512];

	SOCKET sock = *(SOCKET*)lpParam;

	while (true) {
		if (recv(sock, recvBuf, sizeof(recvBuf), 0) == SOCKET_ERROR) {
			errorCall("Recevie message");
			return -1;
		}
		else {
			std::cout << recvBuf << std::endl;
		}
		memset(recvBuf, 0, sizeof(recvBuf));
	}
	return 0;
}

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET ClientSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, hints;

	// wsastartup
	LogCall((WSAStartup(MAKEWORD(2, 2), &wsaData)), "WSA Start Up");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//getaddrinfo
	LogCall(getaddrinfo("localhost", PORT, &hints, &result), "Get Address Information");

	struct addrinfo* ptr = NULL;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		//socket
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			errorCall("ClientSocket created");
		}
		//connect
		int iResult = connect(ClientSocket, ptr->ai_addr, ptr->ai_addrlen);
		if(iResult == SOCKET_ERROR){
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
			if (ptr->ai_next == NULL) {
				errorCall("Server connect");
				return 1;
			}
			continue;
		}
		else {
			std::cout << "Connect to server.\n";
		}
		break;
	}

	DWORD threadID;

	// Create Thread
	HANDLE t1 = CreateThread(NULL, 0, sendMessage, &ClientSocket, 0, &threadID);
	HANDLE t2 = CreateThread(NULL, 0, recvMessage, &ClientSocket, 0, &threadID);

	if (t1 == NULL || t2 == NULL) {
		errorCall("Create thread");
		return 1;
	}
   
	//Wait for single object
	int x = WaitForSingleObject(t1, INFINITE);
	if(x != 0) WaitForSingleObject(t2, INFINITE);
	
	//shutdown
	LogCall(shutdown(ClientSocket, SD_BOTH), "Shut Down");

	closesocket(ClientSocket);
	WSACleanup();
	return 0;

} 
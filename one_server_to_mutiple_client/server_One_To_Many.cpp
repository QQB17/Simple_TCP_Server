#define _WIN32_WINNT 0x501

#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "lws2_32")

#define PORT "24000"

#define errorCall(x) std::cout << x << " failed with " << WSAGetLastError() << std::endl
#define successCall(x) std::cout << x << " success.\n"
#define ASSERT(x) if(x) return 1 
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
				
int main(int argc, char* argv[]) {
	WSADATA wsaData;
	SOCKET ClientSocket = INVALID_SOCKET, ListenSocket = INVALID_SOCKET;
	struct addrinfo* result = nullptr, hints;

	//wsastartup
	LogCall((WSAStartup(MAKEWORD(2, 2), &wsaData)), "WSA Start Up");
		
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // either AF_UNSPEC for TCP or UDP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // ip for tcp
	hints.ai_flags = AI_PASSIVE; // return socket address

	//getaddrinfo
	LogCall((getaddrinfo(NULL, PORT, &hints, &result)), "Get Address information");

	//socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		errorCall("Listen socke build");
		WSACleanup();
		return 1;
	}

	//bind
	LogCall((bind(ListenSocket, result->ai_addr, result->ai_addrlen)), "Bind address");

	//Listen
	LogCall((listen(ListenSocket, SOMAXCONN)), "Listening");

	fd_set master;
	FD_ZERO(&master); // initialize master
	FD_SET(ListenSocket, &master); // declare master

	//accept
	//send & recv
	while (true) {
		fd_set copy = master;

		int socketCount = select(0, &copy, NULL, NULL, NULL);
		for (int i = 0;i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];
			if (sock == ListenSocket) {
				ClientSocket = accept(ListenSocket, NULL, NULL);
				FD_SET(ClientSocket, &master);

				std::cout << "Socket # " << ClientSocket << " is connected to sever.\n";
				const char* message = "Welcome to chat\n";
				send(ClientSocket, message, sizeof(message), 0);
			}
			else {
				char recvBuf[512];
				memset(recvBuf, 0, sizeof(recvBuf));

				int bytesIn = recv(sock, recvBuf, sizeof(recvBuf), 0);
				if (bytesIn <= 0) {
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					for (int j = 0;j < master.fd_count;j++) {
						SOCKET outSock = master.fd_array[j];
						if (outSock != ListenSocket && outSock != sock) {
							std::ostringstream ss;
							ss << "Sock #" << sock << ": " << recvBuf << std::endl;
							std::string sendBuf = ss.str();
							send(outSock, sendBuf.c_str(), sendBuf.size(), 0);
						}
					}
				}
			}
		}
	}

	closesocket(ListenSocket);

	//shutdown
	LogCall((shutdown(ClientSocket, SD_BOTH)), "Shut Down");

	//clear
	WSACleanup();
	return 0;
}
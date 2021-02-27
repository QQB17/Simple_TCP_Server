all:
	g++ client.cpp -o client -lws2_32
	g++ server.cpp -o server -lws2_32
	start cmd.exe /k "server.exe"
	start cmd.exe /k "client.exe"
	
all:
	g++ one_To_Many_client.cpp -o m_client -lws2_32
	g++ server_One_To_Many.cpp -o m_server -lws2_32
	start cmd.exe /k "m_server.exe"
	start cmd.exe /k "m_client.exe"
	start cmd.exe /k "m_client.exe"
	
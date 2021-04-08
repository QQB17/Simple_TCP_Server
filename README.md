# Simple_TCP_Server
Simple C++ TCP server &amp; client by win32 API

## Run program
### Visual Studio PowerShell/Command Prompt
```
$ cd <file>
$ nmake
```

## Window Command Prompt
server.exe
```
$ cd <file>
$ g++ server.cpp -o server -lws2_32
```
client.exe
```
$ cd <file>
$ g++ client.cpp -o client -lws2_32
```
#### Make sure to run the server.exe before running client.exe
---

## References:
- Socket Programming : https://www.geeksforgeeks.org/socket-programming-cc/
- Tcp Server : https://www.youtube.com/watch?v=WDn-htpBlnU

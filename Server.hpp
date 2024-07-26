#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")



class Server
{

private:
	WSADATA _ws;
	SOCKET _server_fd;
	fd_set _master;
	struct sockaddr_in _serverAddr;
	static constexpr int PORT = 8080;
	static constexpr int BUFFERSIZE = 4096;

public:
	Server();
	void DisplayActiveClients();
	void Broadcast(SOCKET client_socket,char buffer[BUFFERSIZE]);
	void AcceptNewConnection(SOCKET socket);
	void DropClient(SOCKET socket);
	void OpenChat();
	void Close();


};


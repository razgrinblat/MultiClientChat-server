#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <unordered_map>
#include <string>
#pragma comment(lib,"ws2_32.lib")

class Server
{

private:
	WSADATA _ws;
	SOCKET _server_fd;
	fd_set _master;
	struct sockaddr_in _serverAddr;
	std::unordered_map<uint32_t, std::string> _users_map;
	static constexpr int PORT = 8080;
	static constexpr int BUFFERSIZE = 1058;
	static constexpr int USERNAME_MAX_SIZE = 30;
	static constexpr int DATA_SIZE = 5;
	

public:
	Server();
	void displayActiveClients();
	std::string displayAllUsernames();
	void broadcast(SOCKET client_socket,char buffer[BUFFERSIZE]);
	void acceptNewConnection(SOCKET socket);
	void acceptNewMessage(SOCKET socket);
	void dropClient(SOCKET socket);
	void openChat();
	void close();


};


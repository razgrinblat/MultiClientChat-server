#include "Server.hpp"


Server::Server()
{
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &_ws) == SOCKET_ERROR)
	{
		std::cout << "WSA Failed to Initialize\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "WSA opened Succesfully\n";
	}
	//Initialize the socket
	_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_server_fd == SOCKET_ERROR)
	{
		std::cout << "Failed to Initialize the socket\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "The socket opened Succesfully\n";
	}

	// Set up the sockaddr_in structure
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(PORT);
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	//Bind the socket to local Port
	int bindResult = bind(_server_fd, (sockaddr*)&_serverAddr, sizeof(_serverAddr));
	if (bindResult == SOCKET_ERROR)
	{
		std::cout << "Failed to bind to local server\n";
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Succesfully bind to local port\n";
	}
	// Listen for incoming connections
	int listenResult = listen(_server_fd, 5);
	if (bindResult == SOCKET_ERROR)
	{
		std::cout << "Failed to listenning to local server\n";
		close();
		
	}
	else
	{
		std::cout << "listenning...\n";
	}
}

void Server::broadcast(SOCKET client_socket ,char buffer[BUFFERSIZE])
{
	if (_users_map[client_socket] == "")
	{
		char* username = buffer + DATA_SIZE;
		_users_map[client_socket] = username;
	}
	for (int i = 0; i < _master.fd_count; i++)
	{
		SOCKET outSocket = _master.fd_array[i];
		if (outSocket != _server_fd && outSocket != client_socket)
		{
			send(outSocket, buffer, BUFFERSIZE, 0);
		}
	}
}

void Server::displayActiveClients()
{
	if (_master.fd_count == 1)
	{
		std::cout << "***No Connected Clients***\n";
		return;
	}
	std::cout << "the connected clients:\n";
	for (int i = 0; i < _master.fd_count; i++)
	{
		if (_master.fd_array[i] != _server_fd)
		{
			int port;
			char address[INET_ADDRSTRLEN];
			struct sockaddr_in address_struct;
			int addr_size = sizeof(address_struct);
			getpeername(_master.fd_array[i], (sockaddr*)&address_struct, &addr_size);
			port = ntohs(address_struct.sin_port);
			inet_ntop(AF_INET, &address_struct.sin_addr, address, sizeof(address));//convert IPv4 addresses from binary to text
			std::cout << "Client ip: " << address << ", Port: " << port << std::endl;
		}
	}
}

std::string Server::displayAllUsernames()
{
	std::string users_msg = "$users$\n";
	for (const auto& x : _users_map)
	{
		users_msg += x.second + std::string("\r\n");
	}
	return users_msg;
}

void Server::acceptNewConnection(SOCKET socket)
{
	
	//Accept a new connection
	SOCKET client = accept(socket, (sockaddr*)&client, nullptr);
	//Add the new connection to the list of clients
	FD_SET(client, &_master);
	displayActiveClients();
	//Send a welcome message
	const char* welcomeMsg = "Welcome to my server\r\ntype 'exit' to close the chat\n\rtype $users$ to see all usernames";
	send(client, welcomeMsg, strlen(welcomeMsg), 0);

	//char binary_size[32];
	//recv(socket, binary_size, 32, 0);
	//int length = std::stoi(binary_size);
	//char* username = new char[length];
	//recv(socket, username, length, 0);
	//_users_map[client] = username; //insert the username into [client FD, name] map

}

void Server::acceptNewMessage(SOCKET sock)
{
	char buf[BUFFERSIZE];
	memset(&buf, 0, BUFFERSIZE);

	int bytes = recv(sock, buf, BUFFERSIZE, 0);
	if (bytes <= 0)
	{
		dropClient(sock);
		return;
	}
	if (strcmp(buf, "exit") == 0)
	{
		dropClient(sock);
		return;
	}
	if (strcmp(buf,"$users$") == 0)
	{
		std::string users_msg = displayAllUsernames();
		int send_result = send(sock, users_msg.c_str(), users_msg.size() + 1, 0);
		if (send_result == SOCKET_ERROR)
		{
			std::cout << "Error in get users name\n";
		}	
	}
	else
	{
		broadcast(sock, buf);
	}
}

void Server::dropClient(SOCKET sock)
{
	FD_CLR(sock, &_master);
	_users_map.erase(sock);
	displayActiveClients();
	closesocket(sock);	
}

void Server::openChat()
{
	FD_ZERO(&_master);  //clear the set
	FD_SET(_server_fd, &_master); //Add the listenning socket to the set
	while (true)
	{
		
		fd_set copy = _master;
		//master contains all the clients file descriptors, copy contains all the actives fd's after calling select()
		int activeSocketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
		if (activeSocketCount == -1)
		{
			std::cout << "Error with Select function";
			close();
		}
		for (int i = 0; i < activeSocketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (FD_ISSET(_server_fd, &copy))
			{
				acceptNewConnection(sock);
			}
			else
			{
				acceptNewMessage(sock);
			}
		}
	}
}

void Server::close()
{
	//close Listenning Socket
	if (closesocket(_server_fd) == SOCKET_ERROR)
	{
		std::cout << "closing failed\n";
	}
	//WSAcleanup
	if (WSACleanup() == SOCKET_ERROR)
	{
		std::cout << "cleenup failed\n";
	}
	exit(EXIT_FAILURE);
}


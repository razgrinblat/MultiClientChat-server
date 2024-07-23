#include "Server.hpp"


Server::Server()
{
	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &_ws) == SOCKET_ERROR)
	{
		std::cout << "WSA Failed to Initialize\n";
		return;
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
		return;
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
		return;
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
		Close();
		return;
	}
	else
	{
		std::cout << "listenning...\n";
	}
}

void Server::Broadcast(SOCKET client_socket ,char buffer[1024])
{
	for (int i = 0; i < _master.fd_count; i++)
	{
		SOCKET outSocket = _master.fd_array[i];
		if (outSocket != _server_fd && outSocket != client_socket)
		{
			send(outSocket, buffer, strlen(buffer), 0);
		}
	}
}

void Server::DisplayActiveClients()
{
	if (_master.fd_count == 1)
	{
		std::cout << "***No Clients Connected***";
		return;
	}
	std::cout << "the connected clients:\n";
	for (int i = 0; i < _master.fd_count; i++)
	{
		if (_master.fd_array[i] != _server_fd)
		{
			std::cout << "socket fd is: " << _master.fd_array[i] << std::endl;
		}
	}
}

void Server::AcceptNewConnection(SOCKET socket)
{
	//Accept a new connection
	SOCKET client = accept(socket, (sockaddr*)&client, nullptr);
	//Add the new connection to the list of clients
	FD_SET(client, &_master);
	DisplayActiveClients();
	//Send a welcome message
	const char* welcomeMsg = "Welcome to the Raz server\r\n";
	send(client, welcomeMsg, strlen(welcomeMsg), 0);
}

void Server::DropClient(SOCKET sock)
{
	closesocket(sock);
	FD_CLR(sock, &_master);
	DisplayActiveClients();
}

void Server::OpenChat()
{
	FD_ZERO(&_master);  //clear the set
	FD_SET(_server_fd, &_master); //Add the listenning socket to the set
	while (true)
	{
		fd_set copy = _master;
		//master contains all the clients file descriptors, copy contains all the actives fd's after calling select()
		int activeSocketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < activeSocketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (FD_ISSET(_server_fd, &copy))
			{
				AcceptNewConnection(sock);
			}
			else
			{
				//Accept a new message
				char buf[BUFFERSIZE];
				memset(&buf, 0, BUFFERSIZE);

				int bytes = recv(sock, buf, BUFFERSIZE, 0);

				if (bytes == 0)
				{
					DropClient(sock);
				}
				else
				{
					Broadcast(sock, buf);
				}
			}
		}
	}
}

void Server::Close()
{
	//close Listenning Socket
	if (closesocket(_server_fd) == SOCKET_ERROR)
	{
		printf("closing failed \n");
	}
	//WSAcleanup
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("cleenup failed \n");
	}
}






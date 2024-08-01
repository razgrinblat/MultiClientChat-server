#include "Server.hpp"

Server::Server()
{
	initializeWinsock();
	initializeSocket();
	bindToPort();
	listenForConnection();
}

Server::~Server()
{
	//close Listenning Socket
	if (closesocket(_server_fd) == SOCKET_ERROR)
	{
		throw std::runtime_error("closing failed\n");
	}
	//WSAcleanup
	if (WSACleanup() == SOCKET_ERROR)
	{
		throw std::runtime_error("cleenup failed\n");
	}
}

void Server::initializeWinsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &_ws) == SOCKET_ERROR)
	{
		throw std::runtime_error("WSA Failed to Initialize\n");
	}
	std::cout << "WSA opened Succesfully\n";
}

void Server::initializeSocket()
{
	//Initialize the socket
	_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_server_fd == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to Initialize the socket\n");
	}
	std::cout << "The socket opened Succesfully\n";

}

void Server::bindToPort()
{
	// Set up the sockaddr_in structure
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(PORT);
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	//Bind the socket to local Port
	int bindResult = bind(_server_fd, (sockaddr*)&_serverAddr, sizeof(_serverAddr));
	if (bindResult == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to bind to local server\n");
	}
	std::cout << "Succesfully bind to local port\n";

}

void Server::listenForConnection()
{
	// Listen for incoming connections
	int listenResult = listen(_server_fd, REQUESTS_QUEUE);
	if (listenResult == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to listenning to local server\n");
	}
	std::cout << "listenning...\n";

}

void Server::displayActiveClients()
{
	if (_master_sockets.fd_count == 1)
	{
		std::cout << "***No Connected Clients***\n";
		return;
	}
	std::cout << "the connected clients:\n";
	for (int i = 0; i < _master_sockets.fd_count; i++)
	{
		if (_master_sockets.fd_array[i] != _server_fd)
		{
			std::array<char, INET_ADDRSTRLEN> address;
			struct sockaddr_in address_struct;
			int addr_size = sizeof(address_struct);
			getpeername(_master_sockets.fd_array[i], (sockaddr*)&address_struct, &addr_size);
			int port = ntohs(address_struct.sin_port);
			inet_ntop(AF_INET, &address_struct.sin_addr, address.data(),address.size());//convert IPv4 addresses from binary to text
			std::cout << "Client ip: " << address.data() << ", Port: " << port << std::endl;
		}
	}
}

std::string Server::displayAllUsernames()
{
	std::string users_msg = "\n";
	for (const auto& x : _users_map)
	{
		users_msg += x.second + std::string("\r\n");
	}
	users_msg += "========================";
	return users_msg;
}

void Server::broadcast(SOCKET client_socket , const std::array<char, BUFFERSIZE> &buffer)
{
	std::string username(buffer.data());
	if (username == USERS_COMMAND)
	{
		send(client_socket, buffer.data(), BUFFERSIZE, 0);
		return;
	}
	if (_users_map[client_socket].empty())
	{
		_users_map[client_socket] = username;
	}
	for (int i = 0; i < _master_sockets.fd_count; i++)
	{
		SOCKET outSocket = _master_sockets.fd_array[i];
		if (outSocket != _server_fd && outSocket != client_socket)
		{
			send(outSocket, buffer.data(), BUFFERSIZE, 0);
		}
	}
}

void Server::acceptNewConnection(SOCKET socket)
{
	//Accept a new connection
	SOCKET client = accept(socket, (sockaddr*)&client, nullptr);
	//Add the new connection to the list of clients
	FD_SET(client, &_master_sockets);
	displayActiveClients();	
	//Send a welcome message
	constexpr auto welcomeMsg = R"(
   ___________________________________________________
  / _________________________________________________ \
 | |        _____  ._.             ._.      ._.      | |
 | |       / ___ \ | |             | |      | |      | |
 | |      | /   \/ | |__.   .___.  | |__.   | |      | |
 | |      | |      | '_. \ /  ^  \ |  __|   | |      | |
 | |      | \___/\ | | | | | (_) | |  |__.  '-'      | |
 | |       \_____/ |_| |_| \___^._\ \____|  [=]      | |
 | |_________________________________________________| |
 | |                                                 | |
 | |   Welcome to the "RazChat!" App.                | |
 | |_________________________________________________| |
 | |                 (c)                             | |
 | |   Copyright 2024   Raz Grinblat.                | |
 | |_________________________________________________| |
 | |                                                 | |
 | |   type $exit$ to close the chat                 | |
 | |   type $users$ to see usernames list            | |
 | |_________________________________________________| |
  \___________________________________________________/
)";
	send(client, welcomeMsg, strlen(welcomeMsg), 0);
}

void Server::acceptNewMessage(SOCKET sock)
{
	std::array<char, BUFFERSIZE> buffer;
	buffer.fill(0);

	int bytes = recv(sock, buffer.data(), BUFFERSIZE, 0);
	if (bytes <= 0)
	{
		dropClient(sock);
		return;
	}
	std::string username = buffer.data();
	if (username.compare(EXIT_MESSAGE_COMMAND) == 0)
	{
		broadcast(sock, buffer);
		dropClient(sock);
		return;
	}
	if (username.compare(USERS_COMMAND) == 0)
	{
		std::string users_msg = displayAllUsernames();
		std::memcpy(buffer.data() + USERNAME_MAX_SIZE + DATA_SIZE, users_msg.c_str(), users_msg.size() + 1);
		broadcast(sock, buffer);
		return;
	}
	else
	{
		broadcast(sock, buffer);
	}
}

void Server::dropClient(SOCKET sock)
{
	FD_CLR(sock, &_master_sockets);
	_users_map.erase(sock);
	displayActiveClients();
	closesocket(sock);	
}

void Server::openChat()
{
	FD_ZERO(&_master_sockets);  //clear the set
	FD_SET(_server_fd, &_master_sockets); //Add the listenning socket to the set
	while (true)
	{
		fd_set copy = _master_sockets;
		//master contains all the clients file descriptors, copy contains all the actives fd's after calling select()
		int activeSocketCount = select(FD_SETSIZE, &copy, nullptr, nullptr, nullptr);
		if (activeSocketCount == SOCKET_ERROR)
		{
			std::cout << "Error with Select function";
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

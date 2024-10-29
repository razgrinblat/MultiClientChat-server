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
			getpeername(_master_sockets.fd_array[i], reinterpret_cast<sockaddr*>(&address_struct), &addr_size);
			int port = ntohs(address_struct.sin_port);
			inet_ntop(AF_INET, &address_struct.sin_addr, address.data(),address.size());//convert IPv4 addresses from binary to text
			std::cout << "Client ip: " << address.data() << ", Port: " << port << std::endl;
		}
	}
}

std::string Server::getUsernamesString()
{
	std::string users_msg = "\n";
	for (const auto& x : _users_map)
	{
		users_msg += x.second + std::string(NEW_LINE);
	}
	users_msg += "*======================*";
	return users_msg;
}

void Server::broadcast(SOCKET client_socket, const std::vector<char>& buffer)
{
	std::string username = buffer.data() + DATA_SIZE;
	if (username == USERS_COMMAND)	
	{
		send(client_socket, buffer.data(), buffer.size(), 0);
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
			send(outSocket, buffer.data(), buffer.size(), 0);
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
	static constexpr auto welcomeMsg = R"(
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

void Server::handleUsesrsCommand(std::vector<char> &buffer, std::string &command, SOCKET socket)
{
	std::array<char, DATA_SIZE + 1> pdu_size;
	std::string users_msg = getUsernamesString();
	std::snprintf(pdu_size.data(), pdu_size.size(), "%04d", users_msg.size() + strlen(USERS_COMMAND) + sizeof(NULL_TERMINATOR) + 1);
	buffer.clear();
	buffer.insert(buffer.end(), pdu_size.begin(), pdu_size.end() - 1);
	buffer.insert(buffer.end(), command.begin(), command.end());
	buffer.push_back('\0');
	buffer.insert(buffer.end(), users_msg.begin(), users_msg.end());
	buffer.push_back('\0');
	broadcast(socket, buffer);
}

void Server::acceptNewMessage(SOCKET sock)
{
	std::array<char, DATA_SIZE + 1> pdu_size_str = {};
	pdu_size_str.fill(0);
	recv(sock, pdu_size_str.data(), DATA_SIZE + 1, 0);

	int pdu_length = std::atoi(pdu_size_str.data());
	std::vector<char> buffer(pdu_length);

	int bytes = recv(sock, buffer.data(), pdu_length, 0);
	if (bytes <= 0)
	{
		dropClient(sock);
		return;
	}
	std::string username = buffer.data() + DATA_SIZE;
	if (username == EXIT_MESSAGE_COMMAND)
	{
		broadcast(sock, buffer);
		dropClient(sock);
		return;
	}
	if (username == USERS_COMMAND)
	{
		handleUsesrsCommand(buffer, username, sock);
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
			throw std::runtime_error("Error with Select function");
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

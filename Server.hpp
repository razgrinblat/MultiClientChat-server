#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <array>
#pragma comment(lib,"ws2_32.lib")

class Server
{

private:
	WSADATA _ws;
	SOCKET _server_fd;
	fd_set _master_sockets;
	struct sockaddr_in _serverAddr;
	std::unordered_map<uint32_t, std::string> _users_map;
	static constexpr int PORT = 8080;
	static constexpr int BUFFERSIZE = 1054;
	static constexpr int USERNAME_MAX_SIZE = 30;
	static constexpr int DATA_SIZE = 4;
	static constexpr int REQUESTS_QUEUE = 5;
	static constexpr auto USERS_COMMAND = "USERS";
	static constexpr auto EXIT_MESSAGE_COMMAND = "EXIT MESSAGE";
	

public:
	Server();
	~Server();
	void initializeWinsock();
	void initializeSocket();
	void bindToPort();
	void listenForConnection();
	/**
 * @brief Displays the IP addresses and ports of all connected clients.
 */
	void displayActiveClients();
	/**
 * @brief Retrieves a formatted list of all usernames connected to the server.
 * @return A string containing the usernames of all connected users.
 */
	std::string displayAllUsernames();
	/**
 * @brief Broadcasts a message to all connected clients, except the sender.
 * @param client_socket The socket of the client sending the message.
 * @param buffer The message buffer to broadcast.
 */
	void broadcast(SOCKET client_socket, const std::array<char, BUFFERSIZE> &buffer);
	/**
 * @brief Accepts a new incoming connection and adds the client to the list.
 * @param socket The socket on which to accept new connections.
 */
	void acceptNewConnection(SOCKET socket);
	/**
 * @brief Processes a client's message.
 * @param sock The client's socket.
 */
	void acceptNewMessage(SOCKET socket);
	/**
 * @brief Removes a client from the server.
 * @param sock The socket of the client to be removed.
 */
	void dropClient(SOCKET socket);
	/**
 * @brief Starts the chat server and handles incoming connections and messages.
 */
	void openChat();
};


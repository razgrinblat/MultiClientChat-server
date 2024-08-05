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
	static constexpr auto PORT = 8080;
	static constexpr auto BUFFERSIZE = 1054;
	static constexpr auto USERNAME_MAX_SIZE = 30;
	static constexpr auto DATA_SIZE = 4;
	static constexpr auto REQUESTS_QUEUE = 5;
	static constexpr auto USERS_COMMAND = "USERS";
	static constexpr auto EXIT_MESSAGE_COMMAND = "EXIT MESSAGE";
	static constexpr auto NEW_LINE = "\r\n";
	static constexpr auto NULL_TERMINATOR = '\0';

/**
* @brief Initializes Winsock.
* @throw std::runtime_error "WSA Failed to Initialize" if Winsock initialization fails.
*/
	void initializeWinsock();
/**
* @brief Initializes the socket for communication.
* @throw std::runtime_error "Failed to Initialize the socket" if socket creation fails.
*/
	void initializeSocket();
/**
 * @brief Binds the server socket to a local port.
 * @throw std::runtime_error "Failed to bind to local server" if binding the socket to the port fails.
 */
	void bindToPort();
/**
 * @brief Listens for incoming connections.
 * @throw std::runtime_error "Failed to listenning to local server" if the server fails to enter the listening state.
 */
	void listenForConnection();
/**
 * @brief Displays the IP addresses and ports of all connected clients.
 */
	void displayActiveClients();
/**
 * @brief Retrieves a formatted list of all usernames connected to the server.
 * @return A string containing the usernames of all connected users.
 */
	std::string getUsernamesString();
/**
 * @brief Broadcasts a message to all connected clients, except the sender.
 * @param client_socket The socket of the client sending the message.
 * @param buffer The message buffer to broadcast.
 */
	void broadcast(SOCKET client_socket, const std::vector<char>& buffer);
/**
 * @brief Accepts a new incoming connection and adds the client to the list.
 * @param socket The socket on which to accept new connections.
 */
	void acceptNewConnection(SOCKET socket);
	
 /**
* @brief Handles the USERS command by preparing and broadcasting a list of usernames.
* @param buffer The buffer used to store the PDU data. 
* @param command The command string("USERS") , indicating the type of operation.
* @param socket The socket descriptor used to broadcast the PDU to the client.
*/
	void handleUsesrsCommand(std::vector<char>& buffer, std::string &command, SOCKET socket);
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
	

public:

	Server();
	~Server();
/**
 * @brief Starts the chat server and handles incoming connections and messages.
 */
	void openChat();
};
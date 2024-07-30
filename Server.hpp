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
	/**
 * @brief Displays the IP addresses and ports of all connected clients.
 *
 * This function prints the IP addresses and ports of all clients currently
 * connected to the server, excluding the server's own listening socket.
 */
	void displayActiveClients();
	/**
 * @brief Retrieves a formatted list of all usernames connected to the server.
 *
 * This function iterates through the map of users and constructs a single
 * string containing all usernames. Each username is followed by a carriage
 * return and newline character pair ("\r\n"), and the string ends with a
 * separator line.
 *
 * @return A string containing the usernames of all connected users.
 */
	std::string displayAllUsernames();
	/**
 * @brief Broadcasts a message to all connected clients, except the sender.
 *
 * This function sends a message to all clients connected to the server, except
 * the client specified by `client_socket`. If the message contains "USERS", it
 * sends the message only back to the requesting client.
 *
 * @param client_socket The socket of the client sending the message.
 * @param buffer The message buffer to broadcast.
 */
	void broadcast(SOCKET client_socket,const char buffer[BUFFERSIZE]);
	/**
 * @brief Accepts a new incoming connection and adds the client to the list.
 *
 * This function accepts a new connection on the specified socket and adds the
 * new client to the list of connected clients. It also sends a welcome message
 * to the new client.
 *
 * @param socket The socket on which to accept new connections.
 */
	void acceptNewConnection(SOCKET socket);
	/**
 * @brief Handles a new message from a client.
 *
 * This function receives a message from a client and processes it. If the
 * message contains "EXIT MESSAGE", the client is disconnected. If the message
 * contains "USERS", the list of all usernames is sent back to the client.
 * Otherwise, the message is broadcast to all other clients.
 *
 * @param sock The socket from which to receive the message.
 */
	void acceptNewMessage(SOCKET socket);
	/**
 * @brief Removes a client from the server.
 *
 * This function removes the specified client from the list of connected clients,
 * closes the client's socket, and updates the list of active clients.
 *
 * @param sock The socket of the client to be removed.
 */
	void dropClient(SOCKET socket);
	/**
 * @brief Starts the chat server and handles incoming connections and messages.
 *
 * This function enters a loop that waits for activity on the sockets in the
 * master file descriptor set. It accepts new connections or processes incoming
 * messages as appropriate.
 */
	void openChat();
	/**
 * @brief Closes the server and cleans up resources.
 *
 * This function closes the server's listening socket and performs any necessary
 * cleanup, including shutting down Winsock.
 */
	void close();


};


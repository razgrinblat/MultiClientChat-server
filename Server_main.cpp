#include "Server.hpp"

int main()
{
	try {
		Server server = Server();
		server.openChat();
		}
	catch (const std::exception& e)
	{
		std::cerr << "Exception catch: " << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
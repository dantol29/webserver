#include "Server.hpp"
#include "ConfigFile.hpp"
#include "webserv.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
		return (1);
	ConfigFile a(argv[1]);
	if (!a.getErrorMessage().empty())
	{
		std::cout << a.getErrorMessage() << std::endl;
		return 0;
	}
	std::cout << a << std::endl;
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
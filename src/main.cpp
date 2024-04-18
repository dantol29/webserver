#include "Server.hpp"
#include "ConfigFile.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
		return (1);
	if (argc == 2)
	{
		ConfigFile a(argv[1]);
	}
	else if (argc == 1)
	{
		// TODO : handle a base config if no file is passed (otherwise open() will fail/leak
	}
	// std::cout << a << std::endl;
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
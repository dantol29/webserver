#include "Server.hpp"
#include "ConfigFile.hpp"

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	if (argc > 2)
	{
		std::cout << "Usage: ./webserv [<config_file>]" << std::endl;
		return (1);
	}
	else if (argc == 2)
	{
		ConfigFile a(argv[1]);
		if (!a.getErrorMessage().empty())
		{
			std::cout << a.getErrorMessage() << std::endl;
			return 0;
		}
		std::cout << a << std::endl;
	}
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
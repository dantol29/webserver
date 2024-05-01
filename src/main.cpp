#include "Server.hpp"
#include "ConfigFile.hpp"

# define CONFIG_FILE_DEFAULT_PATH "./config"

int main(int argc, char **argv)
{
	ConfigFile a;

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}
	if (argc == 2)
		a.parse(argv[1]);
	else
		a.parse(CONFIG_FILE_DEFAULT_PATH);
	if (!a.getErrorMessage().empty())
	{
		std::cout << a.getErrorMessage() << std::endl;
		return 0;
	}
	// std::cout << a << std::endl; // should be in the DEBUG?
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
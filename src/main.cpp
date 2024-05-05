#include "Server.hpp"
#include "Config.hpp"
#include "webserv.hpp"

#define CONFIG_FILE_DEFAULT_PATH "./webserv.conf"

int main(int argc, char **argv)
{
	Config configInstance;

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}
	if (argc == 2)
		configInstance.parse(argv[1]);
	else
		configInstance.parse(CONFIG_FILE_DEFAULT_PATH);
	if (!configInstance.getErrorMessage().empty())
	{
		std::cout << configInstance.getErrorMessage() << std::endl;
		return 0;
	}
	// std::cout << configInstance << std::endl; // should be in the DEBUG?
	Server webserv(configInstance);
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
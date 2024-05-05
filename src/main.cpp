#include "Server.hpp"
#include "Config.hpp"
#include "webserv.hpp"

#define CONFIG_FILE_DEFAULT_PATH "./webserv.conf"

int main(int argc, char **argv)
{
	Config config;

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}
	if (argc == 2)
		config.parse(argv[1]);
	else
		config.parse(CONFIG_FILE_DEFAULT_PATH);
	if (!config.getErrorMessage().empty())
	{
		std::cout << config.getErrorMessage() << std::endl;
		return 0;
	}
	// std::cout << config << std::endl; // should be in the DEBUG?
	Server webserv(config);
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
#include "Server.hpp"
#include "Config.hpp"
#include "webserv.hpp"

int main(int argc, char **argv)
{

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}

	Config config;
	std::ifstream configFile;

	try{
		if (argc == 2)
		{
			configFile.open(argv[1]);
			config.parse(configFile);
		}
		else
		{
			configFile.open(CONFIG_FILE_DEFAULT_PATH);
			config.parse(configFile);
		}
		if (!config.getErrorMessage().empty())
		{
			std::cout << config.getErrorMessage() << std::endl;
			return 1;
		}
	}
	catch (const char* error){
		std::cout << "Exception caught: " << error << std::endl;
		return 1;
	}
	// std::cout << config << std::endl; // should be in the DEBUG?
	Server webserv(config);
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
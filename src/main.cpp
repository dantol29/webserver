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

	Config config(argv[1]);
	if (!config.getErrorMessage().empty())
		return 1;
	
	//std::cout << config << std::endl; // should be in the DEBUG?
	Debug::enable(true);
	Debug::setLevel(Debug::NORMAL);
	Server webserv(config);
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
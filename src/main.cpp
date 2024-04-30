#include "Server.hpp"
#include "ConfigFile.hpp"
#include "webserv.hpp"

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	// if (argc > 2)
	// 	return (1);
	// ConfigFile a(argv[1]);
	// std::cout << a << std::endl;
	test_debug();
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
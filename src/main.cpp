#include "Server.hpp"
#include "ConfigFile.hpp"
#include <iostream>
#include <ctime>

std::clock_t startTime = std::clock();

int main(int argc, char **argv)
{
	if (argc > 2)
		return (1);
	ConfigFile a(argv[1]);
	std::cout << a << std::endl;
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
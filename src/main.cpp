#include "Server.hpp"
#include "ConfigFile.hpp"

int main(int argc, char **argv)
{
	ConfigFile a;

	if (argc > 2)
		return (1);
	if (argc == 2)
		a.parse(argv[1]);
	else
		a.parse(NULL);
	std::cout << a << std::endl;
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
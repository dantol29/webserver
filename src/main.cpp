#include "Server.hpp"

int main()
{
	Server webserv;
	webserv.startListen();
	webserv.startPollEventLoop();

	return 0;
}
#include "Server.hpp"

#define EXIT_FAILURE 1

int main()
{
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
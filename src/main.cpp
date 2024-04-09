#include "Server.hpp"

int main()
{
	Server webserv;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
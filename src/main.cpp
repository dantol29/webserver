#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include <poll.h>
#include <vector>
#include "webserv.hpp"
#include "Server.hpp"

int main()
{
	Server webserv;
	webserv.startListen();
	webserv.startPollEventLoop();

	return 0;
}
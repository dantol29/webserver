#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "webserv.hpp"
#include "ConfigFile.hpp"

const int PORT = 8080;

int main(int argc, char **argv)
{
	if (argc > 2)
		return (1);
	ConfigFile a(argv[1]);
	if (!a.getErrorMessage().empty()){
		std::cout << a.getErrorMessage() << std::endl;
		return (1);
	}
	std::map<std::string, std::string> var = a.getVariables();
	std::map<std::string, std::string>::iterator it;

	for (std::map<std::string, std::string>::iterator it = var.begin(); it != var.end(); ++it)
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
    int serverFD;
    struct sockaddr_in address;
    int addrLen = sizeof(address);

    // Creating socket file descriptor
    if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

	int opt = 1;
	// Set SO_REUSEADDR to allow re-binding to the same address and port

	if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Try to set SO_REUSEPORT
	if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
		// Don't exit on failure; it's not critical for basic server functionality
		// Setting SO_REUSEPORT is not supported on all systems and setting it on the same call was causing the server
		// to fail
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	ft_memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(serverFD, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(serverFD, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
      while (1) {
        std::cout << "++++++++++++++ Waiting for new connection +++++++++++++++" << std::endl;
        int newSocket = accept(serverFD, (struct sockaddr *)&address, (socklen_t*)&addrLen);
        if (newSocket < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        handleConnection(newSocket);
    }
    return 0;
}
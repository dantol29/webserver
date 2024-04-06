#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"

const int PORT = 8080;

void *ft_memset(void *ptr, int value, size_t num)
{
	// Cast the pointer to a char pointer, as we're dealing with bytes
	unsigned char *p = static_cast<unsigned char *>(ptr);

	// Fill the specified memory area with the given value
	for (size_t i = 0; i < num; ++i)
	{
		p[i] = static_cast<unsigned char>(value);
	}

	// Return the original pointer
	return ptr;
}

int main()
{
	int server_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	// Set SO_REUSEADDR to allow re-binding to the same address and port

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Try to set SO_REUSEPORT
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
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

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("In bind");
		exit(EXIT_FAILURE);
	}
	// 10 is the maximum size of the queue of pending connections: check this value.
	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		std::cout << "++++++++++++++ Waiting for new connection +++++++++++++++" << std::endl;
		int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		if (new_socket < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}
		handleConnection(new_socket);
	}
	return 0;
}
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include "HTTPRequest.hpp"
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

const char *returnHTML()
{
	return "HTTP/1.1 200 OK\nContent-Type: text/html\n"
		   "Content-Length: 198\n\n"
		   "<!DOCTYPE html>"
		   "<html>"
		   "<head>"
		   "<style>"
		   "h1 {"
		   "  background-color: yellow;"
		   "  color: lightblue;"
		   "  border: 2px solid pink;"
		   "  padding: 10px;"
		   "  text-align: center;"
		   "}"
		   "</style>"
		   "</head>"
		   "<body>"
		   "<h1>Hello, Webserv!</h1>"
		   "</body>"
		   "</html>";
}

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
	int server_fd, new_socket;
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	char buffer[BUFFER_SIZE] = {0};

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
	if (listen(server_fd, 10) < 0)
	{
		perror("In listen");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		{
			perror("In accept");
			exit(EXIT_FAILURE);
		}

		valread = read(new_socket, buffer, BUFFER_SIZE);
		HTTPRequest obj(buffer);
		std::cout << obj.getStatusCode() << std::endl;
		std::map<std::string, std::string> storage = obj.getStorage();
		std::map<std::string, std::string>::iterator it =  storage.begin();
		
		while (it != storage.end()){
			std::cout << "key: " << it->first << ", value: " << it->second << std::endl;
			it++;
		}
		if (valread < 0)
		{
			perror("In read");
			exit(EXIT_FAILURE);
		}
		std::cout << "Received http request: " << std::endl << buffer << std::endl;
		std::cout << buffer << std::endl;
		// Respond to the request with some HTML
		write(new_socket, returnHTML(), strlen(returnHTML()));
		std::cout << "------------------HTML message sent-------------------" << std::endl;

		close(new_socket);
	}
	return 0;
}

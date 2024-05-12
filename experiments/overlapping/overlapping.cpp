#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <iostream>

#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

int main()
{
	int server_fd_all, server_fd_specific, client_socket;
	struct sockaddr_in address_all, address_specific;
	int opt = 1;
	int port = 8080;

	// Create socket file descriptor for all IPs
	if ((server_fd_all = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Socket created for all IPs." << std::endl;
	}

	// Forcefully attaching socket to the port 8080 for all IPs
	if (setsockopt(server_fd_all, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address_all.sin_family = AF_INET;
	address_all.sin_addr.s_addr = INADDR_ANY;
	address_all.sin_port = htons(port);

	if (bind(server_fd_all, (struct sockaddr *)&address_all, sizeof(address_all)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd_all, MAX_CLIENTS) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Listening on port " << port << " for all IPs." << std::endl;
	}

	// Create socket file descriptor for specific IP
	if ((server_fd_specific = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Socket created for specific IP." << std::endl;
	}

	// Forcefully attaching socket to the port 8080 for specific IP
	if (setsockopt(server_fd_specific, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address_specific.sin_family = AF_INET;
	address_specific.sin_addr.s_addr = inet_addr("127.0.0.1");
	address_specific.sin_port = htons(port);

	if (bind(server_fd_specific, (struct sockaddr *)&address_specific, sizeof(address_specific)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd_specific, MAX_CLIENTS) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "Listening on port " << port << " for specific IP." << std::endl;
	}

	// Setup poll
	struct pollfd fds[2];
	int nfds = 2, timeout = 5000; // Timeout of 5000 ms

	fds[0].fd = server_fd_all;
	fds[0].events = POLLIN;
	fds[1].fd = server_fd_specific;
	fds[1].events = POLLIN;

	char buffer[BUFFER_SIZE];
	const char *http_ok = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World!";

	std::cout << "Waiting for connections..." << std::endl;

	while (true)
	{
		std::cout << "Polling..." << std::endl;
		int ret = poll(fds, nfds, timeout);
		std::cout << "Poll returned: " << ret << std::endl;

		if (ret < 0)
		{
			perror("poll");
			break;
		}
		if (ret == 0)
		{
			std::cout << "Timeout occurred! No data within 5 seconds." << std::endl;
			continue;
		}

		// if (fds[0].revents & POLLIN)
		// {
		// 	std::cout << "Connection request on All IPs socket." << std::endl;
		// 	// You can accept the connection here and handle accordingly
		// }

		// if (fds[1].revents & POLLIN)
		// {
		// 	std::cout << "Connection request on Specific IP socket." << std::endl;
		// 	// You can accept the connection here and handle accordingly
		// }
		for (int i = 0; i < nfds; i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (i == 0)
				{
					std::cout << "Connection request on All IPs socket." << std::endl;
				}
				else
				{
					std::cout << "Connection request on Specific IP socket." << std::endl;
				}
				socklen_t addrlen = sizeof(address_all);
				client_socket = accept(fds[i].fd, (struct sockaddr *)&address_all, &addrlen);
				if (client_socket < 0)
				{
					perror("accept");
					exit(EXIT_FAILURE);
				}
				else
				{
					std::cout << "Connection accepted." << std::endl;
				}
				memset(buffer, 0, BUFFER_SIZE);
				read(client_socket, buffer, BUFFER_SIZE);
				std::cout << "Received: " << buffer << std::endl;
				send(client_socket, http_ok, strlen(http_ok), 0);
				close(client_socket);
			}
		}
	}

	close(server_fd_all);
	close(server_fd_specific);

	return 0;
}

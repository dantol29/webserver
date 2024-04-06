#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include <poll.h>
#include <vector>
#include "include/webserv.hpp"

const int PORT = 8080;

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
	std::vector<struct pollfd> fds;
	// Create and initailize the pollfd structure for the server socket
	// struct pollfd fds[1];
	// fds[0].fd = server_fd;
	// fds[0].events = POLLIN;
	struct pollfd server_fd_poll;
	server_fd_poll.fd = server_fd;
	// We are listening for incoming connections, i.e. the socket will be readable
	server_fd_poll.events = POLLIN;
	server_fd_poll.revents = 0;
	fds.push_back(server_fd_poll);
	// Set server socket to non-blocking
	// Not necessary since we are using poll
	// int flags = fcntl(server_fd, F_GETFL, 0);
	// fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

	while (1)
	{
		std::cout << "++++++++++++++ Waiting for new connection +++++++++++++++" << std::endl;
		// int ret = poll(fds, 1, -1); // -1 means wait indefinitely
		// fds.data() returns a pointer to the underlying array of pollfd structures
		int ret = poll(fds.data(), fds.size(), -1); // -1 means wait indefinitely
		// if the server socket is readable, then a new connection is available
		if (ret > 0)
		{
			// We loop over all the file descriptors in the pollfd structure
			for (size_t i = 0; i < fds.size(); i++)
			{
				if (fds[i].revents & POLLIN)
				{

					// Check if the server socket is readable: fds[0] is the server socket
					if (i == 0)
					{
						std::cout << "New connection detected" << std::endl;
						// accept is blocking but we know that the server socket is readable
						int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
						if (new_socket >= 0)
						{
							// Add the new socket to the pollfd structure
							struct pollfd new_socket_poll;
							new_socket_poll.fd = new_socket;
							new_socket_poll.events = POLLIN;
							new_socket_poll.revents = 0;
							fds.push_back(new_socket_poll);
						}
						else
						{
							perror("In accept");
							// TODO: Decide how to handle accept failure: exit, continue, or handle gracefully
						}
					}
					else
					{
						// Handle the client socket
						handleConnection(fds[i].fd);
						// Remove the client socket from the pollfd structure
						fds.erase(fds.begin() + i);
					}
				}
				else if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					if (i == 0)
					{
						// Handle the server socket error
						// Log the error if necessary
						// Attempt recovery or initiate a graceful shutdown
						// Possibly alert administrators
						perror("poll server socket error");
						// exit(EXIT_FAILURE);
					}
					else
					{
						// Handle the client socket error
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						perror("poll client socket error");
						--i; // Adjust the index to account for the removed element
							 // exit(EXIT_FAILURE);
					}
				}
			}
		}
		else if (ret == 0)
		{
			std::cout << "Timeout occurred!" << std::endl; // This should never happen with an infinite timeout
		}
		else
		{
			// handle the errors on poll on the server socket
			if (errno == EINTR)
			{
				// poll was interrupted by a signal
				// Log the interruption if necessary
				continue; // Retry the poll operation
			}
			else
			{
				// poll failes: EBADF, EFAULT, EINVAL, ENOMEM
				// Log critical error details
				// Attempt recovery or initiate a graceful shutdown
				// Possibly alert administrators
				perror("poll");
				// exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}
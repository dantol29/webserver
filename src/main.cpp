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

const int PORT = 8080;

int main()
{
	Server webserv;

	std::vector<struct pollfd> fds;
	// Create and initailize the pollfd structure for the server socket
	// struct pollfd fds[1];
	// fds[0].fd = server_fd;
	// fds[0].events = POLLIN;
	struct pollfd serverFdPoll;
	serverFdPoll.fd = serverFD;
	// We are listening for incoming connections, i.e. the socket will be readable
	serverFdPoll.events = POLLIN;
	serverFdPoll.revents = 0;
	fds.push_back(serverFdPoll);
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
						int new_socket = accept(serverFD, (struct sockaddr *)&address, (socklen_t *)&addrLen);
						if (new_socket >= 0)
						{
							// Add the new socket to the pollfd structure
							struct pollfd newSocketPoll;
							newSocketPoll.fd = new_socket;
							newSocketPoll.events = POLLIN;
							newSocketPoll.revents = 0;
							fds.push_back(newSocketPoll);
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
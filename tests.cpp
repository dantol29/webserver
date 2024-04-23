#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <sys/wait.h>
#include <cstring>
#include <poll.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define POLL_TIMOUT 2000

// ADD TO THE MAIN IN WEBSERVER (after read())
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;

// ./webserver
// c++ tests.cpp -o test
// ./test "test_name"

void evalSendOutput(ssize_t bytesSent, const char *request, int clientSocket)
{
	size_t requestLength = strlen(request);

	if (bytesSent < 0)
		std::cerr << "Failed to send data." << std::endl;
	else if (bytesSent == 0 && requestLength > 0)
		std::cerr << "Unexpected scenario: No data was sent despite a request being available." << std::endl;
	else if (bytesSent == 0 && requestLength == 0)
		std::cerr << "You tried to send an empty request. Why?" << std::endl;
	else if (bytesSent < requestLength)
		std::cerr << "Request was sent successfully, but not all data was sent." << std::endl;
	else if (bytesSent > 0 && requestLength == bytesSent)
	{
		std::cout << "Request sent successfully" << std::endl;
		return;
	}
	else
		std::cerr << "Something impossible happened." << std::endl;

	// For all error cases, we close the socket.
	std::cerr << "Request: " << request << std::endl;
	close(clientSocket);
}

bool waitForResponseWitPoll(int socketFd, int timoutMilliseconds)
{
	struct pollfd pollFd;
	pollFd.fd = socketFd;
	pollFd.events = POLLIN;
	pollFd.revents = 0;
	std::cout << "Waiting for response..." << std::endl;
	int ret = poll(&pollFd, 1, timoutMilliseconds);
	if (ret < 0)
	{
		std::cerr << "Poll failed" << std::endl;
		return false;
	}
	else if (ret == 0)
	{
		std::cerr << "Poll timed out" << std::endl;
		return false;
	}
	else if (pollFd.revents & POLLIN)
	{
		return true;
	}
	else
	{
		std::cerr << "Poll returned an unexpected value" << std::endl;
		return false;
	}
}

void sendData(const char *requests[], sockaddr_in serverAddress)
{
	int numRequests = 0;

	// Count the number of requests
	while (requests[++numRequests])
	{
	}
	std::cout << "Number of requests: " << numRequests << std::endl;
	// Create a socket for each request
	int clientSockets[numRequests];

	for (int i = 0; i < numRequests; i++)
	{
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Processing request #" << i + 1 << std::endl;
		std::cout << "Request: " << requests[i] << std::endl;
		// Create a socket
		// clientSockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if ((clientSockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			std::cerr << "Socket creation failed" << std::endl;
			continue;
		}
		// Connect to the server
		if (connect(clientSockets[i], (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		{
			std::cerr << "Connection failed" << std::endl;
			close(clientSockets[i]);
			continue;
		}
		// Send the request
		ssize_t bytesSent = send(clientSockets[i], requests[i], strlen(requests[i]), 0);
		// Evaluate the output of the send function
		evalSendOutput(bytesSent, requests[i], clientSockets[i]);
		// Wait and read the response
		char buffer[BUFFER_SIZE];
		if (!waitForResponseWitPoll(clientSockets[i], POLL_TIMOUT))
		{
			std::cerr << "No response was received" << std::endl;
		}
		else
		{

			ssize_t bytesRead = read(clientSockets[i], buffer, BUFFER_SIZE);
			if (bytesRead < 0)
				std::cerr << "Failed to read data." << std::endl;
			else if (bytesRead == 0)
				std::cerr << "No data was read." << std::endl;
			else
			{
				buffer[bytesRead] = '\0';
				std::cout << "Response: " << buffer << std::endl;
			}
		}
		close(clientSockets[i]);
		sleep(2);
		std::cout << "--------------------------------" << std::endl;
	}
}

void simple(sockaddr_in serverAddress)
{
	const char *requests[] = {"GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n",	   // 200 (OK)
							  "POST / HTTP/1.1\r\nHost: www.example.com\r\n\r\n",	   //  200 (OK)
							  "GETT / HTTP/1.1\r\nHost: www.example.com\r\n\r\n",	   // 501 (Not Implemented)
							  "GET /random HTTP/1.1\r\nHost: www.example.com\r\n\r\n", // 400 (Bad Request)
							  "GET / HTTP/9.9s\r\nHost: www.example.com\r\n\r\n",	   // 400 (Bad Request)
							  " / HTTP/1.1\r\nHost: www.example.com\r\n\r\n",		   // 400 (Bad Request)
							  "GET / HTTP/1.1\nHost: www.example.com\r\n\r\n",		   // 400 (Bad Request)
							  NULL};
	sendData(requests, serverAddress);
}

void query(sockaddr_in serverAddress)
{
	const char *requests[] = {
		"GET /search?q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n",  // 200 (OK)
		"GET /search?q==now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", //  400 (Bad Request)
		"GET /search??q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", // 400 (Bad Request)
		"GET /search?now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n",	  // 400 (Bad Request)
		"GET /search?q=now&&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", // 400 (Bad Request)
		"GET /search?q=now&price=low= HTTP/1.1\r\nHost: www.example.com\r\n\r\n", // 400 (Bad Request)
		"GET /search?=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n",	  // 400 (Bad Request)
		"GET /search?&q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", // 400 (Bad Request)
		NULL};
	sendData(requests, serverAddress);
}

void headers(sockaddr_in serverAddress)
{
	const char *requests[] = {"GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n",				  // 200 (OK)
							  "GET / HTTP/1.1\r\nHost: www.example.com\r\nSecond: hello\r\n\r\n", // 200 (OK)
							  "GET / HTTP/1.1\r\nRandom: www.example.com\r\n\r\n",				  //  400 (Bad Request)
							  "GET / HTTP/1.1\r\nHost www.example.com\r\n\r\n",					  // 400 (Bad Request)
							  "GET / HTTP/1.1\r\nHost:: www.example.com\r\n\r\n",				  // 400 (Bad Request)
							  "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r",					  // 400 (Bad Request)
							  "GET / HTTP/1.1\r\nHost:www.example.com\r\n\r\n",					  // 400 (Bad Request)
							  "GET / HTTP/1.1\r\n Host: www.example.com\r\n\r\n",				  // 400 (Bad Request)
							  "GET /HTTP/1.1\r\nHo st: www.example.com\r\n\r\n",				  // 400 (Bad Request)
							  "GET / HTTP/1.1\r\nHost: www.example.com\nSecond: hello\r\n\r\n",	  // 400 (Bad Request)
							  NULL};
	sendData(requests, serverAddress);
}

void body(sockaddr_in serverAddress)
{
	const char *requests[] = {
		"POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		"text/plain\r\n\r\nThis\r\nis body\r\n\r\n", // 200 (OK)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\r\nis body\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\r\nis body\r\n\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\rThis\r\nis body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\ris body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\r\n\r\nis body\r\n\r\n", // 400 (Bad Request)
		//   "GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\r\nis body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\n\r\nThis\r\nis "
		//   "body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: text/plain\r\n\r\nThis\r\nis "
		//   "body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 20\r\nContent-Type: "
		//   "text/notplain\r\n\r\nThis\r\nis body\r\n\r\n", // 400 (Bad Request)
		//   "POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: abcd\r\nContent-Type: "
		//   "text/plain\r\n\r\nThis\r\nis body\r\n\r\n", // 400 (Bad Request)
		NULL};
	sendData(requests, serverAddress);
}

int main(int argc, char **argv)
{

	if (argc != 2)
	{
		std::cout << "At least 1 argument!" << std::endl;
		return (1);
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (std::strcmp(argv[1], "query") == 0)
		query(serverAddress);
	else if (std::strcmp(argv[1], "simple") == 0)
		simple(serverAddress);
	else if (std::strcmp(argv[1], "headers") == 0)
		headers(serverAddress);
	else if (std::strcmp(argv[1], "body") == 0)
		body(serverAddress);
	else
		std::cout << "Invalid test name" << std::endl;
}
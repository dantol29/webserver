#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <poll.h>

#define BUFFER_SIZE 1024
#define PORT 8080
#define POLL_TIMOUT 2000
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

// ADD TO THE MAIN IN WEBSERVER (after read())
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;

// ./webserver
// c++ tests.cpp -o test
// ./test "test_name"

struct HTTPTest
{
	std::string request;
	std::string expectedResponse;

	HTTPTest(std::string request, std::string expectedResponse) : request(request), expectedResponse(expectedResponse)
	{
	}
};

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

void sendData(const std::vector<HTTPTest> &tests, sockaddr_in serverAddress)
{
	for (size_t i = 0; i < tests.size(); ++i)
	{
		const auto &test = tests[i];
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Processing request #" << i + 1 << std::endl;
		std::cout << "Request: " << test.request << std::endl;
		int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (clientSocket < 0)
		{
			std::cerr << "Socket creation failed" << std::endl;
			continue;
		}
		if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress) < 0))
		{
			std::cerr << "Connection failed" << std::endl;
			close(clientSocket);
			continue;
		}
		ssize_t bytesSent = send(clientSocket, test.request.c_str(), test.request.size(), 0);

		char buffer[BUFFER_SIZE];
		if (waitForResponseWitPoll(clientSocket, POLL_TIMOUT))
		{
			ssize_t bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
			if (bytesRead < 0)
				std::cerr << "Failed to read data." << std::endl;
			else if (bytesRead == 0)
				std::cerr << "No data was read." << std::endl;
			else
			{
				buffer[bytesRead] = '\0';
				if (std::string(buffer) == test.expectedResponse)
				{
					std::cout << "Response: " << buffer << std::endl;
					std::cout << COLOR_GREEN "✅ Test Passed" COLOR_RESET << std::endl;
				}
				else
				{
					std::cerr << "Response: " << buffer << std::endl;
					std::cerr << COLOR_RED "❌ Test Failed" COLOR_RESET << std::endl;
				}
			}
		}
		else
		{
			std::cerr << "No response was received" << std::endl;
		}
		close(clientSocket);
		sleep(1);
		std::cout << "--------------------------------" << std::endl;
	}
}

void simple(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"),
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"),
		HTTPTest("GETT / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "501"),
		HTTPTest("GET /random HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/9.9s\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest(" / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\nHost: www.example.com\r\n\r\n", "400"),
	};
	sendData(tests, serverAddress);
}

void query(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		HTTPTest("GET /search?q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"),
		HTTPTest("GET /search?q==now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search??q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search?now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search?q=now&&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search?q=now&price=low= HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search?=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /search?&q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
	};
	sendData(tests, serverAddress);
}

void headers(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"),
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\nSecond: hello\r\n\r\n", "200"),
		HTTPTest("GET / HTTP/1.1\r\nRandom: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost:: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\n\r", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost:www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\n Host: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /HTTP/1.1\r\nHo st: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\nSecond: hello\r\n\r\n", "400"),
	};
	sendData(tests, serverAddress);
}

void body(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "200"),
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n",
				 "400"), // 400 (Bad Request) -- - Wrong content length // This case is
						 // complicated: we have an extra linera issue for it!}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\n",
				 "400"), // 400 (Bad Request) - - Improper line termination of the body with '\n'}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\rThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Malformed headers (misplaced 'r')}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\ris body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Malformed headers (misplaced 'n') -- // TODO : why is this invalid?}
		HTTPTest(
			"POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
			"text/plain\r\n\r\nThis\r\n\r\nis body\r\n\r\n",
			"400"), // 400 (Bad Request) -- - Improper line termination of the body // with '\r' // TODO: are you sure?}
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - GET request with body}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\n\r\nThis\r\nis "
				 "body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Missing content type}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: text/plain\r\n\r\nThis\r\nis "
				 "body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Missing content length}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/notplain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Invalid content type}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: abcd\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Invalid content length value}

	};
	sendData(tests, serverAddress);
}

int main(int argc, char **argv)
{

	if (argc != 2)
	{
		std::cout << "Incorrect usage!\n" << std::endl;
		std::cout << "Usage: " << argv[0] << " <test_name>" << std::endl;
		std::cout << "Available test names: query, simple, headers, body" << std::endl;
		return 1; // Returning 1 to indicate an error condition
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
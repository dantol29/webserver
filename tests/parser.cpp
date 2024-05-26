#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <poll.h>
#include <sstream>
#include <regex>

#define BUFFER_SIZE 1024
#define PORT 8080
#define POLL_TIMOUT 2000
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"
#define PRINT_RESPONSE 0

// ADD TO THE MAIN IN WEBSERVER (after read())
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;

bool is_error = false;

struct HTTPTest
{
	std::string request;
	std::string expectedResponse;

	HTTPTest(std::string request, std::string expectedResponse) : request(request), expectedResponse(expectedResponse)
	{
	}
};

std::string extractStatusCode(const std::string &response)
{
	std::regex statusLineRegex(R"(HTTP/\d+\.\d+\s+(\d+)\s+.*\r\n)");
	std::smatch matches;
	if (std::regex_search(response, matches, statusLineRegex) && matches.size() > 1)
	{
		return matches[1].str(); // The first sub-match is the status code
	}
	else
	{
		std::cerr << "Invalid or malformed HTTP response." << std::endl;
		return "";
	}
	std::istringstream responseStream(response);
	std::string httpVersion;
	std::string statusCode;

	responseStream >> httpVersion >> statusCode;

	return statusCode;
}

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
		if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
		{
			std::cerr << "Connection failed" << std::endl;
			close(clientSocket);
			continue;
		}
		ssize_t bytesSent = send(clientSocket, test.request.c_str(), test.request.size(), 0);

		char buffer[BUFFER_SIZE];
		if (waitForResponseWitPoll(clientSocket, POLL_TIMOUT * 15))
		{
			ssize_t bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
			if (bytesRead < 0)
				std::cerr << "Failed to read data." << std::endl;
			else if (bytesRead == 0)
				std::cerr << "No data was read." << std::endl;
			else
			{
				buffer[bytesRead] = '\0';
				std::string statusCode = extractStatusCode(buffer);
				if (statusCode == test.expectedResponse)
				{
					if (PRINT_RESPONSE)
						std::cout << "Response: " << buffer << std::endl;
					std::cout << "Status code: " << statusCode << std::endl;
					std::cout << COLOR_GREEN "✅ Test Passed" COLOR_RESET << std::endl;
				}
				else
				{
					if (PRINT_RESPONSE)
						std::cout << "Response: " << buffer << std::endl;
					std::cout << "Status code: " << statusCode << std::endl;
					std::cerr << COLOR_RED "❌ Test Failed" COLOR_RESET << std::endl;
					is_error = true;
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
		// TODO: HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"),
		HTTPTest("GETT / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "501"),
		HTTPTest("GET /random HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "404"),
		HTTPTest("GET / HTTP/9.9s\r\nHost: www.example.com\r\n\r\n", "505"),
		HTTPTest(" / HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "501"),
		HTTPTest("GET / HTTP/1.1\nHost: www.example.com\r\n\r\n", "400"),
	};
	sendData(tests, serverAddress);
}

void query(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		// HTTPTest("GET /index.html?q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "200"), // TODO:
		// Implement query parsing
		HTTPTest("GET /index.html?q==now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html??q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html?now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html?q=now&&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html?q=now&price=low= HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html?=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
		HTTPTest("GET /index.html?&q=now&price=low HTTP/1.1\r\nHost: www.example.com\r\n\r\n", "400"),
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
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\n\r", "408"),
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
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 100\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "408"),// 408 (Timeout) -- - Wrong content length
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\n",
				 "200"), // 200 body shouldn't end with CRLF
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\rThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Malformed headers (misplaced 'r')}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 16\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\ris body\r\n\r\n",
				 "200"),
		HTTPTest("GET / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "200"), //  GET request with body is correct
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\n\r\nThis\r\nis "
				 "body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Missing content type}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: text/plain\r\n\r\nThis\r\nis "
				 "body\r\n\r\n",
				 "411"), // 400 (Bad Request) -- - Missing content length}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: 17\r\nContent-Type: "
				 "text/notplain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "415"), // 400 (Bad Request) -- - Invalid content type}
		HTTPTest("POST / HTTP/1.1\r\nHost: www.example.com\r\nContent-Length: abcd\r\nContent-Type: "
				 "text/plain\r\n\r\nThis\r\nis body\r\n\r\n",
				 "400"), // 400 (Bad Request) -- - Invalid content length value}

	};
	sendData(tests, serverAddress);
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	// if (argc != 2)
	// {
	// 	std::cout << "Incorrect usage!\n" << std::endl;
	// 	std::cout << "Usage: " << argv[0] << " <test_name>" << std::endl;
	// 	std::cout << "Available test names: query, simple, headers, body" << std::endl;
	// 	return 1; // Returning 1 to indicate an error condition
	// }

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	// if (std::strcmp(argv[1], "query") == 0)
	std::cout << "\033[38;5;214mRunning query tests\033[0m" << std::endl;
	query(serverAddress);
	// else if (std::strcmp(argv[1], "simple") == 0)
	std::cout << "\033[38;5;214mQuery tests done\033[0m" << std::endl;
	std::cout << "\033[38;5;214mRunning simple tests\033[0m" << std::endl;
	simple(serverAddress);
	std::cout << "\033[38;5;214mSimple tests done\033[0m" << std::endl;
	std::cout << "\033[38;5;214mRunning headers tests\033[0m" << std::endl;
	// else if (std::strcmp(argv[1], "headers") == 0)
	headers(serverAddress);
	std::cout << "\033[38;5;214mHeaders tests done\033[0m" << std::endl;
	// else if (std::strcmp(argv[1], "body") == 0)
	body(serverAddress);
	// else
	// std::cout << "Invalid test name" << std::endl;
	if (is_error)
		exit(1);
	else if (!is_error)
		std::cout << std::endl << "     🎉 All tests passed 🎉" << std::endl << std::endl;
	exit(0);
}
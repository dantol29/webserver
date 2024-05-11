#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
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
#define PRINT_RESPONSE 1

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

void salad(sockaddr_in serverAddress)
{
	std::vector<HTTPTest> tests = {
		HTTPTest("GET / HTTP/1.1\r\nHost: www.saladbook.xyz\r\n\r\n", "200"),
		HTTPTest("GET /story/our_story.html HTTP/1.1\r\nHost: www.saladbook.xyz\r\n\r\n", "200"),
		HTTPTest("GET / HTTP/1.1\r\nHost: saladbook.xyz\r\n\r\n", "404"),
		HTTPTest("GET /fsdfsd HTTP/1.1\r\nHost: www.saladbook.xyz\r\n\r\n", "400"),
		HTTPTest("GET / HTTP/1.1\r\nHost: www.sgfgdf\r\n\r\n", "404"),
		HTTPTest("GET / HTXXXP/1.1\r\nHost: www.sgfgdf\r\n\r\n", "400"),

	};
	sendData(tests, serverAddress);
}

int main(void)
{
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	salad(serverAddress);

	if (is_error)
		exit(1);
	else if (!is_error)
		std::cout << std::endl << "     🎉 All tests passed 🎉" << std::endl << std::endl;
	exit(0);
}
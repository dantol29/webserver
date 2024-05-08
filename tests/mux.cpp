#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <mutex>
#include <vector>
#include <regex>
#include <sstream>

std::mutex coutMutex;

using NameUrlPair = std::pair<std::string, std::string>;

// Struct to hold the results of each request
struct RequestResult
{
	std::string name;
	double duration; // in milliseconds
};

// Function to extract the status code from the HTTP response
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
}

RequestResult GetRequest(const NameUrlPair &nameUrlPair)
{
	const std::string &name = nameUrlPair.first;
	const std::string &url = nameUrlPair.second;

	std::lock_guard<std::mutex> lock(coutMutex);
	std::cout << "--------------------------------" << std::endl;
	std::cout << "Processing request: " << name << std::endl;
	std::cout << "Request URL: " << url << std::endl;

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "Socket creation failed" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
		std::cout << "--------------------------------" << std::endl;
		return {name, -1};
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
	{
		std::cerr << "Invalid address / Address not supported" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
		close(sock);
		std::cout << "--------------------------------" << std::endl;
		return {name, -1};
	}

	if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		std::cerr << "Connection failed" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
		close(sock);
		std::cout << "--------------------------------" << std::endl;
		return {name, -1};
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::string request = "GET " + url + " HTTP/1.1\r\nHost: development_site\r\n\r\n";
	if (send(sock, request.c_str(), request.length(), 0) < 0)
	{
		std::cerr << "Failed to send request" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
		close(sock);
		std::cout << "--------------------------------" << std::endl;
		return {name, -1};
	}

	char buffer[4096];
	ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
	{
		std::cerr << "Failed to receive response" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
	}
	else if (bytesReceived == 0)
	{
		std::cerr << "No data received" << std::endl;
		std::cout << "❌ Test Failed" << std::endl;
	}
	else
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;
		std::string response(buffer, bytesReceived);
		size_t headerEnd = response.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			std::string statusCode = extractStatusCode(response);
			std::cout << "Response Header:" << std::endl << response.substr(0, headerEnd) << std::endl;
			std::cout << "Status Code: " << statusCode << std::endl;
			std::cout << "✅ Test Passed. Time taken: " << duration.count() << " ms" << std::endl;
			std::cout << "--------------------------------" << std::endl;
			close(sock);
			return {name, duration.count()};
		}
		else
		{
			std::cerr << "Failed to find end of response header" << std::endl;
			std::cout << "❌ Test Failed" << std::endl;
		}
	}

	std::cout << "--------------------------------" << std::endl;
	close(sock);
	return {name, -1};
}

int main()
{
	std::vector<NameUrlPair> nameUrlPairs = {
		{"1 - Index", "/index.html"}, {"2 - Largefile", "/largefile"}, {"3 - Example", "/example.html"}};

	std::thread threads[3];
	std::vector<RequestResult> results(3);

	for (size_t i = 0; i < nameUrlPairs.size(); ++i)
	{
		threads[i] = std::thread([&results, i, &nameUrlPairs]() { results[i] = GetRequest(nameUrlPairs[i]); });
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	// Verify the last request was faster than the second (largefile)
	if (results[2].duration >= 0 && results[1].duration >= 0 && results[2].duration < results[1].duration)
	{
		std::cout << "✅ Test Passed: The third request was faster than the second." << std::endl;
		return 0;
	}
	else
	{
		std::cout << "❌ Test Failed: The third request was not faster than the second." << std::endl;
		return 1;
	}
}

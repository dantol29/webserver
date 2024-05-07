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

// Mutex for cout (otherwise output gets mixed up)
std::mutex coutMutex;

// Define a pair type for name and URL
using NameUrlPair = std::pair<std::string, std::string>;

// Function to make a GET request using socket communication
void GetRequest(const NameUrlPair &nameUrlPair)
{
	const std::string &name = nameUrlPair.first;
	const std::string &url = nameUrlPair.second;

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "Socket creation failed\n";
		return;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
	{
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			std::cout << name << std::endl;
			std::cerr << "Invalid address/ Address not supported" << std::endl << std::endl;
		}
		close(sock);
		return;
	}

	if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			std::cout << name << std::endl;
			std::cerr << "Connection failed" << std::endl << std::endl;
		}
		close(sock);
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::string request = "GET " + url + " HTTP/1.1\r\nHost: development_site\r\n\r\n";
	if (send(sock, request.c_str(), request.length(), 0) < 0)
	{
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			std::cout << name << std::endl;
			std::cerr << "Failed to send request" << std::endl << std::endl;
		}
		close(sock);
		return;
	}

	char buffer[4096];
	ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
	{
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			std::cout << name << std::endl;
			std::cerr << "Failed to receive response" << std::endl << std::endl;
		}
	}
	else if (bytesReceived == 0)
	{
		std::cerr << "No data received\n";
	}
	else
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			std::cout << name << std::endl;
			std::cout << "Connected. Response received. Time taken: " << duration.count() << " ms" << std::endl;

			std::string response(buffer, bytesReceived);
			size_t headerEnd = response.find("\r\n\r\n");
			if (headerEnd != std::string::npos)
			{
				std::cout << "Response Header:" << std::endl << response.substr(0, headerEnd) << std::endl;
			}
			else
			{
				std::cerr << "Failed to find end of response header\n";
			}
			std::cout << std::endl << "------------------------------------------" << std::endl << std::endl;
		}
	}

	close(sock);
}

int main()
{
	std::vector<NameUrlPair> nameUrlPairs = {
		{"1 - Index", "/index.html"}, {"2 - Largefile", "/largefile"}, {"3 - example ", "/example.html"}};

	std::thread threads[3];
	for (size_t i = 0; i < nameUrlPairs.size(); ++i)
	{
		// Add a delay before starting each thread to ensure sequential access
		if (i > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Adjust the delay as required
		}
		threads[i] = std::thread(GetRequest, nameUrlPairs[i]);
	}

	// Join threads
	for (auto &thread : threads)
	{
		thread.join();
	}

	return 0;
}

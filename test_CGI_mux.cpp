#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// Function to make a GET request using socket communication
void GetRequest(const std::string &url, const std::string &name)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "Socket creation failed\n";
		return;
	}

	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080); // Assuming HTTP requests on port 80
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
	{
		std::cerr << "Invalid address/ Address not supported\n";
		close(sock);
		return;
	}

	if (connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
	{
		std::cerr << "Connection failed\n";
		close(sock);
		return;
	}

	auto start = std::chrono::high_resolution_clock::now();

	std::string request = "GET " + url + " HTTP/1.1\r\nHost: localhost\r\n\r\n";
	if (send(sock, request.c_str(), request.length(), 0) < 0)
	{
		std::cerr << "Failed to send request\n";
		close(sock);
		return;
	}

	char buffer[4096];
	ssize_t bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
	{
		std::cerr << "Failed to receive response\n";
	}
	else if (bytesReceived == 0)
	{
		std::cerr << "No data received\n";
	}
	else
	{
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end - start;
		std::cout << "Request Name: " << name << std::endl;
		std::cout << "Connected. Response received. Time taken: " << duration.count() << " ms" << std::endl;

		// Find the end of the response header
		std::string response(buffer, bytesReceived);
		size_t headerEnd = response.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			std::cout << "Response Header:\n" << response.substr(0, headerEnd) << std::endl;
		}
		else
		{
			std::cerr << "Failed to find end of response header\n";
		}
		std::cout << "------------------------------------------" << std::endl;
	}

	close(sock);
}

int main()
{
	std::string urls[] = {
		"http://localhost:8080", "http://localhost:8080/development_site/largefile", "http://localhost:8080"};
	std::string names[] = {"Homepage", "Large File", "Another Page"};

	std::thread threads[3];
	for (int i = 0; i < 3; ++i)
	{
		threads[i] = std::thread([=]() { GetRequest(urls[i], names[i]); });
	}

	// Join threads
	for (auto &thread : threads)
	{
		thread.join();
	}

	return 0;
}

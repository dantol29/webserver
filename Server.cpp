#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"
#include "HTTPRequest.hpp"

const int BUFFER_SIZE = 1024;
const size_t MAX_HEADER_SIZE = 8192; // 8KB - This is the limit of the header size also in NGINX

bool isChunked(const std::string &headers)
{
	// Look for "Transfer-Encoding: chunked" in the headers
	// This would not work cause headers are case insensitive
	// std::string search = "Transfer-Encoding: chunked";
	// return headers.find(search) != std::string::npos;
	std::string lowerHeaders;
	std::transform(headers.begin(), headers.end(), std::back_inserter(lowerHeaders), ::tolower);
	std::string search = "transfer-encoding: chunked";
	std::string::size_type pos = lowerHeaders.find(search);
	if (pos != std::string::npos)
	{
		return true;
	}
	return false;
}

size_t getContentLength(const std::string &headers)
{
	std::string lowerHeaders;
	std::transform(headers.begin(), headers.end(), std::back_inserter(lowerHeaders), ::tolower);
	std::string search = "content-length: ";
	std::string::size_type pos = lowerHeaders.find(search);
	if (pos != std::string::npos)
	{
		std::string contentLengthLine = headers.substr(pos + search.size());
		std::string::size_type endPos = contentLengthLine.find("\r\n");
		std::string contentLengthStr = contentLengthLine.substr(0, endPos);
		try
		{
			return std::stoul(contentLengthStr);
		}
		catch (const std::invalid_argument &e)
		{
			std::cerr << "Invalid argument: " << e.what() << '\n';
			return 0; // Or use another way to indicate an error
		}
		catch (const std::out_of_range &e)
		{
			std::cerr << "Out of range: " << e.what() << '\n';
			return 0; // Or use another way to indicate an error
		}
	}
	return 0;
}

bool readLine(int socket, std::string &line)
{
	line.clear();
	while (true)
	{
		char buffer;
		ssize_t bytesRead = recv(socket, &buffer, 1, 0);
		if (bytesRead > 0)
		{
			line.push_back(buffer);
			if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n")
			{
				line.resize(line.size() - 2); // remove the CRLF
				return true;
			}
		}
		else if (bytesRead < 0)
		{
			perror("recv failed");
			return false;
		}
		else
		{
			std::cout << "Connection closed" << std::endl;
			return false;
		}
	}
	return true;
}

std::string readChunk(int socket, size_t chunkSize)
{
	std::string chunkData;
	// Reserve space in the string to avoid reallocations
	chunkData.reserve(chunkSize);
	while (chunkSize > 0)
	{
		char buffer[BUFFER_SIZE];
		size_t bytesToRead = std::min(chunkSize, (size_t)BUFFER_SIZE);
		ssize_t bytesRead = recv(socket, buffer, bytesToRead, 0);
		if (bytesRead > 0)
		{
			chunkData.append(buffer, bytesRead);
			chunkSize -= bytesRead;
		}
		else if (bytesRead < 0)
		{
			perror("recv failed in readChunk");
			throw std::runtime_error("recv failed in readChunk");
		}
		else
		{
			std::cout << "Connection closed while reading chunk" << std::endl;
			throw std::runtime_error("Connection closed while reading chunk");
		}
	}
	char crlf[2];
	ssize_t crlfRead = recv(socket, crlf, 2, 0);
	if (crlfRead < 2)
	{
		std::cout << "Connection closed while reading CRLF" << std::endl;
		throw std::runtime_error("Connection closed while reading CRLF");
	}
	return chunkData;
}

// Determine the type of request and call the appropriate handler
void handleConnection(int socket)
{
	std::string headers;
	size_t totalRead = 0;
	bool headersComplete = false;
	while (!headersComplete)
	{
		// We reinitialize it at each iteration to have a clean buffer
		char buffer[BUFFER_SIZE] = {0};
		// we could do recv non blocking with MSG_DONTWAIT but we will keep it simple for now
		ssize_t bytesRead = recv(socket, buffer, BUFFER_SIZE, 0);
		if (bytesRead > 0)
			headers.append(buffer, bytesRead);
		totalRead += bytesRead;
		if (totalRead > MAX_HEADER_SIZE)
		{
			std::cout << "Header too large" << std::endl;
			// This would be a 413 Payload Too Large in a real server
			close(socket);
			return;
		}
		if (headers.find("\r\n\r\n") != std::string::npos)
			headersComplete = true;
		else if (bytesRead < 0)
		{
			perror("recv failed");
			close(socket);
			return;
			// We could eventually also brake the loop and retry the recv
			// break;
			// We could eventually retry the recv, but for now we will just close the connection
			// Handle error
		}
		else
		{
			// This means biyeRead == 0
			std::cout << "Connection closed" << std::endl;
			close(socket);
			return;
		}
	}
	std::string body;
	if (isChunked(headers))
	{
		// TODO: check if this is blocking, I mean the recv in readChunk
		std::string body;
		bool bodyComplete = false;
		while (!bodyComplete)
		{
			// Chunk Structure: size in hex, CRLF, chunk data, CRLF
			// chunkSizeLine will contain the size of the next chunk in hexadecimal
			std::string chunkSizeLine;
			// Read the line containing the size of the next chunk
			readLine(socket, chunkSizeLine);
			// We transform the size from hexadecimal to an integer
			size_t chunkSize = std::stoul(chunkSizeLine, 0, 16);

			if (chunkSize == 0)
			{
				bodyComplete = true;
			}
			else
			{
				// Read the chunk of data
				std::string chunkData = readChunk(socket, chunkSize); // Implement this function
				body.append(chunkData);
				// Consume the CRLF at the end of the chunk
			}
			// Now, body contains the full body of the request
		}
	}
	else
	{
		size_t contentLength = getContentLength(headers);
		char buffer[BUFFER_SIZE];
		size_t bytesRead = 0;
		while (bytesRead < contentLength)
		{
			// TODO: check if this is blocking
			ssize_t read = recv(socket, buffer, BUFFER_SIZE, 0);
			if (read > 0)
			{
				body.append(buffer, read);
				bytesRead += read;
			}
			else if (read < 0)
			{
				perror("recv failed");
				// Consiger if we should close the socket or retry the recv
				close(socket);
				return;
			}
			else
			{
				std::cout << "Connection closed" << std::endl;
				close(socket);
				return;
			}
		}
	}
	// It should be double "\r\n" to separate the headers from the body
	std::string httpRequestString = headers + "\r\n\r\n" + body;

	HTTPRequest obj(httpRequestString.c_str());
	// HTTPRequest obj(buffer);
	std::cout << obj.getStatusCode() << std::endl;
	std::cout << "Received HTTP request: " << std::endl << httpRequestString << std::endl;

	// test to execute the python script (see: https://www.tutorialspoint.com/python/python_cgi_programming.htm)
	const char *argv[] = {"./cgi-bin/hello_py.cgi", NULL};
	// const char* argv[] = { "./cgi-bin/thirty_py.cgi", NULL };
	// const char* argv[] = { "./cgi-bin/hello.cgi", NULL };

	std::string response;
	if (strstr(buffer, "GET / HTTP/1.1") || strstr(buffer, "GET /home HTTP/1.1"))
	{
		response = handleHomePage();
	}
	else if (strstr(buffer, "GET /hello HTTP/1.1"))
	{
		// env has to be created before CGI, because it is passed to the CGI
		Environment env;
		env.setVar("QUERY_STRING", "Hello from C++ CGI!");
		response = handleCGIRequest(argv, env);
	}
	else
	{
		response = handleNotFound();
	}

	write(socket, response.c_str(), response.size());
	close(socket);
}
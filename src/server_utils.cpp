#include "server_utils.hpp"

void perrorAndExit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

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

bool readChunkSize(int socket, std::string &line)
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

bool readChunk(int socket, size_t chunkSize, std::string &chunkData, HTTPResponse &response)
{
	// Reserve space in the string to avoid reallocations
	chunkData.reserve(chunkSize + chunkData.size());
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
			// Internal Server Error
			response.setStatusCode(500);
			return false;
		}
		else
		{
			// bytes read == 0, connection closed prematurely
			std::cout << "Connection closed while reading chunk" << std::endl;
			response.setStatusCode(400); // Bad Request
			return false;
		}
	}
	char crlf[2];
	ssize_t crlfRead = recv(socket, crlf, 2, 0);
	if (crlfRead < 2)
	{
		std::cout << "Connection closed while reading CRLF" << std::endl;
		response.setStatusCode(400); // Bad Request
		return false;
	}
	return true;
}

void printVariablesHeadersBody(const HTTPRequest &obj)
{
	std::multimap<std::string, std::string> a = obj.getHeaders();
	std::multimap<std::string, std::string> b = obj.getQueryString();
	std::vector<std::string> c = obj.getBody();

	std::multimap<std::string, std::string>::iterator it;
	std::cout << "Variables: =>" << std::endl;
	for (it = b.begin(); it != b.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	std::cout << "Headers: =>" << std::endl;
	for (it = a.begin(); it != a.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	std::cout << "Body: =>" << std::endl;
	for (size_t i = 0; i < c.size(); ++i)
		std::cout << c[i] << std::endl;
}
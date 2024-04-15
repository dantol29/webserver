#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <iomanip>	// std::hex, std::setw, std::setfill
#include <string>

void printHex(const std::string &data)
{
	std::cout << "Hexadecimal representation:" << std::endl;
	for (unsigned int i = 0; i < data.size(); ++i)
		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
}

void printHexStartLength(const std::string &data, size_t start, size_t length)
{
	for (size_t i = start; i < start + length && i < data.size(); ++i)
	{
		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)data[i] << " ";
	}
	std::cout << std::dec << std::endl;
}

void printHexThird(const std::string &data, size_t start, size_t length)
{
	std::cout << "Hexadecimal output from position " << start << ":" << std::endl;
	std::cout << "start: " << start << ", length: " << length << std::endl;
	std::cout << "data.size(): " << data.size() << std::endl;
	size_t end = (start + length <= data.size()) ? start + length : data.size() - start;
	std::cout << "end: " << end << std::endl;
	char *data_c = (char *)data.c_str();
	std::cout << "while loop" << std::endl;
	while (*data_c != '\0')
	{
		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)*data_c << " ";
		std::cout << "[" << *data_c << "] ";

		data_c++;
	}

	std::cout << std::dec << std::endl;
	std::cout << "for loop" << std::endl;
	for (size_t i = start; i < end; ++i)
	{
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)(unsigned char)data[i] << " ";
		if (data[i] == '\r')
			std::cout << "(CR) ";

		if (data[i] == '\n')
			std::cout << "(LF) ";
	}
	std::cout << std::dec << std::endl;
}

void printHTTPRequest(const std::string httpRequest, size_t startPos = 0)
{
	std::cout << "HTTP Request with not printables:" << std::endl;
	for (size_t i = startPos; i < httpRequest.length(); ++i)
	{
		unsigned char c = httpRequest[i];
		{
			switch (c)
			{
			case '\n':
				std::cout << "\\n";
				break;
			case '\r':
				std::cout << "\\r";
				break;
			case '\t':
				std::cout << "\\t";
				break;
			default:
				if (c >= 32 && c <= 126)
					std::cout << c;
				else
					std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
			}
		}
	}
	std::cout << std::endl;
}

int extractContentLength(const std::string &httpRequest)
{
	size_t startPos = httpRequest.find("Content-Length: ");
	std::cout << "startPos: " << startPos << std::endl;
	printHTTPRequest(httpRequest, startPos);
	if (startPos == std::string::npos)
	{
		std::cout << "Content-Length not found in the request" << std::endl;
		return -1; // Content-Length not found
	}
	// startPos += 16; // Move to the end of "Content-Length: "
	std::string contentLengthLiteral = "Content-Length: ";
	startPos += contentLengthLiteral.length(); // Move to the end of "Content-Length: "
	std::cout << "Start pos: " << startPos << std::endl;
	printHTTPRequest(httpRequest, startPos);
	size_t endPos = httpRequest.find("\r\n", startPos);
	std::cout << "endPos: " << endPos << std::endl;
	printHTTPRequest(httpRequest, endPos);
	if (endPos == std::string::npos)
	{
		std::cout << "Malformed HTTP header, no CRLF after Content-Length" << std::endl;
		return -1; // Malformed HTTP header
	}
	std::string contentLengthStr = httpRequest.substr(startPos, endPos - startPos);
	try
	{
		return std::stoi(contentLengthStr);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to parse Content-Length: " << e.what() << std::endl;
		return -1;
	}
}
int extractContentLengthAlt(const std::string &httpRequest)
{
	std::cout << "Extracting Content-Length from HTTP header ..." << std::endl;
	size_t startPos = httpRequest.find("Content-Length:");
	if (startPos == std::string::npos)
	{
		std::cout << "Content-Length not found in the request." << std::endl;
		return -1;
	}
	std::cout << "startPos: " << startPos << std::endl;
	printHTTPRequest(httpRequest, startPos);
	startPos += 15; // Move past "Content-Length:"
	// std::cout << "startPos after moving: " << startPos << std::endl;
	printHTTPRequest(httpRequest, startPos);
	while (startPos < httpRequest.size() && std::isspace(httpRequest[startPos]))
		startPos++; // Skip any spaces after the colon
	// std::cout << "startPos after skipping spaces: " << startPos << std::endl;
	size_t endPos = httpRequest.find("\r\n", startPos);
	std::cout << "endPos: " << endPos << std::endl;
	printHTTPRequest(httpRequest, endPos);
	if (endPos == std::string::npos)
	{
		std::cout << "Malformed HTTP header, no CRLF after Content-Length." << std::endl;
		printHTTPRequest(httpRequest, startPos);
		// printHexStartLength(httpRequest, startPos, 50);
		printHexThird(httpRequest, startPos, 50);
		return -1;
	}

	std::string contentLengthStr = httpRequest.substr(startPos, endPos - startPos);
	std::cout << "Substring for Content-Length: '" << contentLengthStr << "'" << std::endl;
	printHex(contentLengthStr); // Print the substring in hex to check for hidden characters

	try
	{
		int contentLength = std::stoi(contentLengthStr);
		std::cout << "Parsed Content-Length: " << contentLength << std::endl;
		return contentLength;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Failed to parse Content-Length: " << e.what() << std::endl;
		return -1;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "No more or less than 2 arguments" << std::endl;
		return (1);
	}

	// create socket by using the socket() system call.
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	// AF_INET: It specifies the IPv4 protocol family.
	// SOCK_STREAM: It defines that the TCP type socket.
	if (clientSocket == -1)
	{
		std::cerr << "Failed to create socket" << std::endl;
		return 1;
	}

	// define the server address
	sockaddr_in serverAddress;			  // data type that is used to store the address of the socket
	serverAddress.sin_family = AF_INET;	  // specifies the IPv4 protocol family
	serverAddress.sin_port = htons(8080); // convert the unsigned int from machine byte order to network byte order
	serverAddress.sin_addr.s_addr = INADDR_ANY; // we don’t want to bind our socket to any particular IP
												// and instead make it listen to all the available IPs.

	// establish a connection with the server specified by the serverAddress argument

	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0)
	{
		std::cerr << "Connection to server failed" << std::endl;
		close(clientSocket);
		return 1;
	}

	// send data to the server
	int bytesSent = send(clientSocket, argv[1], strlen(argv[1]), 0);
	if (bytesSent < 0)
	{
		std::cerr << "Failed to send data to server" << std::endl;
		close(clientSocket);
		return 1;
	}

	// Shutdown the sending side of the socket to signal no more data will be sent
	shutdown(clientSocket, SHUT_WR);

	// Optionally, wait for a response from the server
	char buffer[1024];
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesReceived > 0)
	{
		std::cout << "Received from server: " << std::string(buffer, bytesReceived) << std::endl;
	}
	else if (bytesReceived == 0)
	{
		std::cout << "Server closed connection" << std::endl;
	}
	else
	{
		std::cerr << "Failed to receive data" << std::endl;
	}
	// Properly close the socket after a response or timeout

	shutdown(clientSocket, SHUT_WR);
	close(clientSocket);

	return 0;
}
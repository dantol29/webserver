#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <iomanip>	// std::hex, std::setw, std::setfill
#include <string>

// Helper function to print characters in hexadecimal
void printHex(const std::string &data)
{
	std::cout << "Hexadecimal representation:" << std::endl;
	// in C++98 without range-based for loop
	for (unsigned int i = 0; i < data.size(); ++i)
	{
		std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
	}

	// for (unsigned char c : data)
	// {
	// 	std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
	// }
	// std::cout << std::dec << std::endl;
}

// Function to print the HTTP request, showing escape characters like \n and \r
void printHTTPRequest(const std::string httpRequest, size_t startPos = 0)
{
	std::cout << "HTTP Request with not printables:" << std::endl;
	for (size_t i = startPos; i < httpRequest.length(); ++i)
	{
		unsigned char c = httpRequest[i];
		{
			switch (c)
			{
			case '\n': // Newline character
				std::cout << "\\n";
				break;
			case '\r': // Carriage return
				std::cout << "\\r";
				break;
			case '\t': // Tab character
				std::cout << "\\t";
				break;
			default:
				if (c >= 32 && c <= 126)
				{
					std::cout << c; // Print all printable ASCII characters
				}
				else
				{
					// Print other non-printables as a hexadecimal value for visibility
					std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
				}
			}
		}
	}
	std::cout << std::endl;
}

// Function to extract content length from HTTP header
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
	std::cout << "startPos after moving: " << startPos << std::endl;
	printHTTPRequest(httpRequest, startPos);
	while (startPos < httpRequest.size() && std::isspace(httpRequest[startPos]))
	{
		startPos++; // Skip any spaces after the colon
	}

	size_t endPos = httpRequest.find("\r\n", startPos);
	std::cout << "endPos: " << endPos << std::endl;
	printHTTPRequest(httpRequest, endPos);
	if (endPos == std::string::npos)
	{
		std::cout << "Malformed HTTP header, no CRLF after Content-Length." << std::endl;
		return -1;
	}

	std::string contentLengthStr = httpRequest.substr(startPos, endPos - startPos);
	std::cout << "Substring for Content-Length: '" << contentLengthStr << "'" << std::endl;
	printHex(contentLengthStr); // Print the substring in hex to check for hidden characters

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

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "No more or less than 2 arguments" << std::endl;
		return (1);
	}

	std::string request = argv[1];
	std::cout << "Request (std::cout): " << request << std::endl;
	printHTTPRequest(request);
	size_t headersBodySeparator = request.find("\r\n\r\n");
	if (headersBodySeparator == std::string::npos)
	{
		std::cerr << "Error: Malformed HTTP request" << std::endl;
		return 1;
	}
	std::string headers(request.substr(0, headersBodySeparator));
	std::string body(request.substr(request.find("\r\n\r\n") + 4));

	// Extract and calculate content length
	// int declaredContentLength = extractContentLength(headers);
	int declaredContentLength = extractContentLengthAlt(headers);
	if (declaredContentLength == -1)
	{
		std::cerr << "Error: Failed to extract Content-Length" << std::endl;
		return 1;
	}
	size_t actualBodyLength = body.length();

	std::cout << "Declared Content-Length: " << declaredContentLength << std::endl;
	std::cout << "Actual Body Length: " << actualBodyLength << std::endl;

	if (declaredContentLength != (int)actualBodyLength)
	{
		std::cerr << "Error: Declared Content-Length does not match the actual body length." << std::endl;
		return 1;
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
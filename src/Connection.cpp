#include "Connection.hpp"
#include <iomanip>

Connection::Connection(struct pollfd &pollFd, Server &server)

{
	(void)server;
	_connectionType = UNDEFINED;
	_pollFd.fd = pollFd.fd;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
	_hasReadSocket = false;
	_hasFinishedReading = false;
	_hasDataToSend = false;
	_hasFinishedSending = false;
	_canBeClosed = false;
}

#if USE_CUSTOM_COPY_CONSTRUCTOR
Connection::Connection(const Connection &other)
{
	_pollFd = other._pollFd;
	_response = other._response;
	_request = other._request;
	_parser = other._parser;

	_connectionType = other._connectionType;

#if USE_COPY_CONSTRUCTOR_WITH_BOOLS
	std::cout << "USE_COPY_CONSTRUCTOR_WIT_BOOLS: " << USE_COPY_CONSTRUCTOR_WITH_BOOLS << std::endl;
	std::cout << "Using copy constructor with bools" << std::endl;
	_hasReadSocket = other._hasReadSocket;
	_hasFinishedReading = other._hasFinishedReading;
	_hasDataToSend = other._hasDataToSend;
	_hasFinishedSending = other._hasFinishedSending;
	_canBeClosed = other._canBeClosed;
#else
	std::cout << "Using copy constructor without bools" << std::endl;
#endif

	std::cout << "Connection object copied" << std::endl;
}
#endif

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_pollFd = other._pollFd;
		_response = other._response;
		// NEW ADDITION
		_request = other._request;
		_parser = other._parser;
		_connectionType = other._connectionType;
		_hasReadSocket = other._hasReadSocket;
		_hasFinishedReading = other._hasFinishedReading;
		_hasDataToSend = other._hasDataToSend;
		_hasFinishedSending = other._hasFinishedSending;
		_canBeClosed = other._canBeClosed;
	}
	std::cout << "Connection object assigned" << std::endl;
	return *this;
}

Connection::~Connection()
{
	// std::cout << "Connection object destroyed" << std::endl;
	// Should I close the file descriptor here?
	// This was causing errors, cause when exiting from scope of the function where the Connection object was creted it
	// was closing the file descriptor. When we push back the Connection object in the vector of connections we make a
	// copy of the object, so the file descriptor was closed in the original object and the copy was left with an
	// invalid file descriptor. if (_pollFd.fd != -1)
	// {
	// 	close(_pollFd.fd);
	// 	_pollFd.fd = -1; // Invalidate the file descriptor to prevent double closing
	// }
}

// GETTERS AND SETTERS

struct pollfd Connection::getPollFd() const
{
	return _pollFd;
}

HTTPResponse &Connection::getResponse()
{
	return _response;
}
HTTPRequest &Connection::getRequest()
{
	return _request;
}

Parser &Connection::getParser()
{
	return _parser;
}

bool Connection::getHasReadSocket() const
{
	return _hasReadSocket;
}

bool Connection::getHasFinishedReading() const
{
	return _hasFinishedReading;
}

bool Connection::getHasDataToSend() const
{
	return _hasDataToSend;
}
bool Connection::getHasFinishedSending() const
{
	return _hasFinishedSending;
}

bool Connection::getCanBeClosed() const
{
	return _canBeClosed;
}

void Connection::setHasReadSocket(bool value)
{
	_hasReadSocket = value;
}

void Connection::setHasFinishedReading(bool value)
{
	std::cout << RED;
	std::cout << "Connection fd: " << _pollFd.fd << std::endl;
	std::cout << "setHasFinishedReading" << std::endl;
	std::cout << RESET;
	_hasFinishedReading = value;
}

void Connection::setHasDataToSend(bool value)
{
	if (value == true)
		_pollFd.events = POLLOUT;
	_hasDataToSend = value;
}

void Connection::setHasFinishedSending(bool value)
{
	if (value == true)
		_pollFd.events = POLLIN;
	_hasFinishedSending = value;
}

void Connection::setCanBeClosed(bool value)
{
	_canBeClosed = value;
}

void Connection::setConnectionType(ConnectionType type)
{
	_connectionType = type;
}
// Attempts to read HTTP request headers from the client connection into _headersBuffer on the Parser.
bool Connection::readHeaders(Parser &parser)
{
	// std::cout << "\nEntering readHeaders" << std::endl;
	char buffer[BUFFER_SIZE] = {0};
	std::cout << "buffers size: " << sizeof(buffer) << std::endl;
	ssize_t bytesRead = recv(_pollFd.fd, buffer, BUFFER_SIZE, 0);
	std::cout << "bytesRead: " << bytesRead << std::endl;
	if (bytesRead > 0)
	{
		parser.setBuffer(parser.getBuffer() + std::string(buffer, bytesRead));
		// std::cout << "The buffer is: " << parser.getBuffer() << std::endl;

		// std::cout << "Exiting readHeaders" << std::endl;
		return true;
	}
	else if (bytesRead < 0)
	{
		perror("recv failed");
		return false;
	}
	else if (bytesRead == 0)
	{
		std::cout << "Connection closed before headers being completely sent" << std::endl;
		return false;
	}
	else
	{
		std::cout << "Exiting readHeaders. This will never happen here!" << std::endl;
		return true;
	}
}
// About the hexa conversion
// Convert the hexadecimal string from `chunkSizeLine` to a size_t value.
// Here, `std::istringstream` is used to create a stream from the string,
// which then allows for input operations similar to cin. The `std::hex`
// manipulator is used to interpret the input as a hexadecimal value.
// We attempt to stream the input into the `chunkSize` variable. If this operation
// fails (e.g., because of invalid input characters that can't be interpreted as hex),
// the stream's failbit is set, and the conditional check fails. In this case,
// we return false indicating an error in parsing the chunk size.

bool Connection::readChunkedBody(Parser &parser)
{
	// TODO: check if this is blocking; I mean the two recvs in readChunkSize and readChunk
	if (!parser.getBodyComplete())
	{
		std::string chunkSizeLine;
		// readChiunkSize cuould be a method of Connection, now it's a free function.
		if (!readChunkSize(chunkSizeLine))
			return false;

		std::istringstream iss(chunkSizeLine);
		size_t chunkSize;
		if (!(iss >> std::hex >> chunkSize))
			return false;

		if (chunkSize == 0)
		{
			parser.setBodyComplete(true);
			return true;
		}
		else
		{
			std::string chunkData;
			if (!readChunk(chunkSize, chunkData, _response))
				return false;
			parser.setBuffer(parser.getBuffer() + chunkData);
		}
	}
	return false;
}

bool Connection::readChunkSize(std::string &line)
{
	// TODO: check this while loop that could be infinite
	// TODO: check if this is blocking

	line.clear();
	while (true)
	{
		char buffer;
		ssize_t bytesRead = recv(_pollFd.fd, &buffer, 1, 0);
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

bool Connection::readChunk(size_t chunkSize, std::string &chunkData, HTTPResponse &response)
{
	// Reserve space in the string to avoid reallocations
	chunkData.reserve(chunkSize + chunkData.size());
	while (chunkSize > 0)
	{
		char buffer[BUFFER_SIZE];
		size_t bytesToRead = std::min(chunkSize, (size_t)BUFFER_SIZE);
		ssize_t bytesRead = recv(_pollFd.fd, buffer, bytesToRead, 0);
		if (bytesRead > 0)
		{
			chunkData.append(buffer, bytesRead);
			chunkSize -= bytesRead;
		}
		else if (bytesRead < 0)
		{
			perror("recv failed in readChunk");
			// Internal Server Error
			response.setStatusCode(500, "");
			return false;
		}
		else
		{
			// bytes read == 0, connection closed prematurely
			response.setStatusCode(400, "Connection closed while reading chunk"); // Bad Request
			return false;
		}
	}
	char crlf[2];
	ssize_t crlfRead = recv(_pollFd.fd, crlf, 2, 0);
	if (crlfRead < 2)
	{
		response.setStatusCode(400, "Connection closed while reading CRLF"); // Bad Request
		return false;
	}
	return true;
}

bool Connection::readBody(Parser &parser, HTTPRequest &req, HTTPResponse &res)
{
	std::cout << "\nEntering readBody" << std::endl;
	size_t contentLength = req.getContentLength();
	char buffer[BUFFER_SIZE];
	// We could also use _bodyTotalBytesRead from the parser
	size_t bytesRead = parser.getBuffer().size();
	std::cout << "contentLength: " << contentLength << std::endl;
	std::cout << "bytesRead: " << bytesRead << std::endl;
	if (bytesRead < contentLength)
	{
		// TODO: check if this is blocking
		ssize_t read = recv(_pollFd.fd, buffer, BUFFER_SIZE, 0);
		if (read > 0)
		{
			std::cout << "read > 0" << std::endl;
			// _body.append(buffer, read);j
			parser.setBuffer(parser.getBuffer() + std::string(buffer, read));
			std::cout << "bytesRead: " << parser.getBuffer().size() << std::endl;
			// std::cout << "The 'body; is: " << parser.getBuffer() << std::endl;
			bytesRead += read;
			if (bytesRead == contentLength)
			{
				parser.setBodyComplete(true);
				return true;
			}
		}
		else if (read < 0)
		{
			perror("recv failed");
			res.setStatusCode(500, ""); // Internal Server Error
			return false;
		}
		else
		{
			std::cout << "read == 0" << std::endl;
			res.setStatusCode(499, "Connection closed by the client"); // Client Closed Request
			return false;
		}
	}
	else
		parser.setBodyComplete(true);
	std::cout << "Exiting readBody" << std::endl;
	return true;
}

/* Debugging */

void Connection::printConnection() const
{
	std::cout << CYAN << "CONNECTION # " << _pollFd.fd << " (fd)" << RESET << std::endl;

	std::string connectionType;

	if (_connectionType == SERVER)
		connectionType = "SERVER";
	else if (_connectionType == CLIENT)
		connectionType = "CLIENT";
	else
		connectionType = "UNDEFINED";

	// Initial padding of 5 spaces and then the property name, with a fixed width for alignment
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "type:" << connectionType << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasReadSocket:" << _hasReadSocket << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasFinishedReading:" << _hasFinishedReading
			  << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasDataToSend:" << _hasDataToSend << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasFinishedSending:" << _hasFinishedSending
			  << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "canBeClosed:" << _canBeClosed << std::endl;
}
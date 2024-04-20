#include "Connection.hpp"

Connection::Connection(struct pollfd &pollFd, Server &server) : _bodyComplete(false), _bodyIsChunked(false)
{
	(void)server;
	_pollFd.fd = pollFd.fd;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
	// TODO: should I initialize the _response here?
	_response = HTTPResponse();
	_bodyComplete = false;
	_bodyIsChunked = false;
	std::cout << "Connection created" << std::endl;
	std::cout << "pollFd.fd: " << _pollFd.fd << std::endl;
	std::cout << "pollFd.events: " << _pollFd.events << std::endl;
	std::cout << "pollFd.revents: " << _pollFd.revents << std::endl;
}

Connection::Connection(const Connection &other)
{
	_pollFd = other._pollFd;
	_body = other._body;
	_response = other._response;
	_bodyComplete = other._bodyComplete;
	_bodyIsChunked = other._bodyIsChunked;
	_chunkData = other._chunkData;

	std::cout << "Connection object copied" << std::endl;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_pollFd = other._pollFd;
		_body = other._body;
		_response = other._response;
		_bodyComplete = other._bodyComplete;
		_bodyIsChunked = other._bodyIsChunked;
		_chunkData = other._chunkData;
	}
	std::cout << "Connection object assigned" << std::endl;
	return *this;
}

Connection::~Connection()
{
	std::cout << "Connection object destroyed" << std::endl;
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

struct pollfd Connection::getPollFd() const
{
	return _pollFd;
}

bool Connection::getBodyComplete() const
{
	return _bodyComplete;
}

HTTPResponse &Connection::getResponse()
{
	return _response;
}

std::string Connection::getBody() const
{
	return _body;
}

std::string Connection::getChunkData() const
{
	return _chunkData;
}

bool Connection::getBodyIsChunked() const
{
	return _bodyIsChunked;
}

void Connection::setBodyComplete(bool bodyComplete)
{
	_bodyComplete = bodyComplete;
}

void Connection::setBody(const std::string &body)
{
	_body = body;
}

void Connection::setChunkData(const std::string &chunkData)
{
	_chunkData = chunkData;
}

void Connection::setBodyIsChunked(bool bodyIsChunked)
{
	_bodyIsChunked = bodyIsChunked;
}

// Attempts to read HTTP request headers from the client connection into _headersBuffer on the Parser.
bool Connection::readSocket(Parser &parser)
{
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytesRead = recv(_pollFd.fd, buffer, BUFFER_SIZE, 0);
	if (bytesRead > 0)
	{
		parser.setBuffer(parser.getBuffer() + std::string(buffer, bytesRead));
		std::cout << "Exiting readSocket" << std::endl;
		return true;
	}
	else if (bytesRead < 0)
	{
		perror("recv failed");
		return false;
	}
	else
	{
		std::cout << "Connection closed before headers being completely sent" << std::endl;
		return false;
	}
	std::cout << "Exiting readSocket. This will never happen here!" << std::endl;
	return true;
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

bool Connection::readChunkedBody()
{
	// TODO: check if this is blocking; I mean the two recvs in readChunkSize and readChunk
	if (!_bodyComplete)
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
			_bodyComplete = true;
			return true;
		}
		else
		{
			std::string chunkData;
			if (!readChunk(chunkSize, chunkData, _response))
				return false;
			_body.append(chunkData);
			// Consume the CRLF at the end of the chunk
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
	ssize_t crlfRead = recv(_pollFd.fd, crlf, 2, 0);
	if (crlfRead < 2)
	{
		std::cout << "Connection closed while reading CRLF" << std::endl;
		response.setStatusCode(400); // Bad Request
		return false;
	}
	return true;
}

bool Connection::readBody(Parser &parser)
{
	std::cout << "\nEntering readBody" << std::endl;
	size_t contentLength = getContentLength(parser.getHeadersBuffer());
	std::cout << "Content-Length: " << contentLength << std::endl;
	char buffer[BUFFER_SIZE];
	// We could also use _bodyTotalBytesRead from the parser
	size_t bytesRead = parser.getBuffer().size();
	std::cout << "bytesRead: " << bytesRead << std::endl;
	_body.append(parser.getBuffer());
	if (bytesRead < contentLength)
	{
		// TODO: check if this is blocking
		ssize_t read = recv(_pollFd.fd, buffer, BUFFER_SIZE, 0);
		if (read > 0)
		{
			std::cout << "read > 0" << std::endl;
			_body.append(buffer, read);
			std::cout << "_body: " << _body << std::endl;
			bytesRead += read;
			if (bytesRead == contentLength)
			{
				_bodyComplete = true;
				return true;
			}
		}
		else if (read < 0)
		{
			perror("recv failed");
			_response.setStatusCode(500); // Internal Server Error
			return false;
		}
		else
		{
			std::cout << "read == 0" << std::endl;
			std::cout << "Connection closed" << std::endl;
			// 400 is not always correct in this case
			// _response.setStatusCode(400); // Bad Request
			return false;
		}
	}
	else
	{
		_bodyComplete = true;
	}
	std::cout << "Exiting readBody" << std::endl;
	_bodyComplete = true;
	return true;
}

bool Connection::isChunked(Parser &parser)
{
	// Look for "Transfer-Encoding: chunked" in the headers
	// This would not work cause headers are case insensitive
	// std::string search = "Transfer-Encoding: chunked";
	// return headers.find(search) != std::string::npos;
	std::string headers = parser.getHeadersBuffer();
	std::string lowerHeaders;
	for (std::string::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		lowerHeaders += customToLower(*it);
	}

	std::string search = "transfer-encoding: chunked";
	std::string::size_type pos = lowerHeaders.find(search);
	if (pos != std::string::npos)
	{
		return true;
	}
	return false;
}

/* Debugging */

void Connection::printConnection() const
{
	std::cout << "\nprintConnection" << std::endl;
	std::cout << "Connection: " << _pollFd.fd << std::endl;
}
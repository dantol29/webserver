#include "Connection.hpp"

Connection::Connection(struct pollfd pollFd, Server &server)
	: _headersComplete(false), _headersTotalBytesRead(0), _bodyComplete(false), _bodyIsChunked(false)
{
	_pollFd.fd = pollFd.fd;
	_pollFd.events = POLLIN | POLLOUT;
	_pollFd.revents = 0;
	memset(&_pollFd, 0, sizeof(_pollFd));
	// TODO: should I initialize the _response here?
	_response = HTTPResponse();
	// TODO: should I initialize the _headers, _body, and _chunkData to empty strings?
	_headers = "";
	_headersComplete = false;
	_headersTotalBytesRead = 0;
	_body = "";
	_chunkData = "";
	_bodyComplete = false;
	_bodyIsChunked = false;
	_clientMaxHeadersSize = server.getClientMaxHeadersSize();
}

Connection::Connection(const Connection &other)
{
	_pollFd = other._pollFd;
	_headers = other._headers;
	_body = other._body;
	_response = other._response;
	_headersComplete = other._headersComplete;
	_bodyComplete = other._bodyComplete;
	_headersTotalBytesRead = other._headersTotalBytesRead;
	_bodyIsChunked = other._bodyIsChunked;
	_chunkData = other._chunkData;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_pollFd = other._pollFd;
		_headers = other._headers;
		_body = other._body;
		_response = other._response;
		_headersComplete = other._headersComplete;
		_bodyComplete = other._bodyComplete;
		_headersTotalBytesRead = other._headersTotalBytesRead;
		_bodyIsChunked = other._bodyIsChunked;
		_chunkData = other._chunkData;
	}
	return *this;
}

Connection::~Connection()
{
	// Should I close the file descriptor here?
	if (_pollFd.fd != -1)
	{
		close(_pollFd.fd);
		_pollFd.fd = -1; // Invalidate the file descriptor to prevent double closing
	}
}

struct pollfd Connection::getPollFd() const
{
	return _pollFd;
}

bool Connection::getHeadersComplete() const
{
	return _headersComplete;
}

bool Connection::getBodyComplete() const
{
	return _bodyComplete;
}

HTTPResponse &Connection::getResponse()
{
	return _response;
}

std::string Connection::getHeaders() const
{
	return _headers;
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

void Connection::setHeadersComplete(bool headersComplete)
{
	_headersComplete = headersComplete;
}

void Connection::setBodyComplete(bool bodyComplete)
{
	_bodyComplete = bodyComplete;
}

void Connection::setBodyIsChunked(bool bodyIsChunked)
{
	_bodyIsChunked = bodyIsChunked;
}

void Connection::setHeaders(const std::string &headers)
{
	_headers = headers;
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

bool Connection::readHeaders()
{
	// Both are in the Connection object now
	// size_t totalRead = 0;
	// bool headersComplete = false;
	// while (!headersComplete)
	if (!_headersComplete)
	{
		// We reinitialize it at each iteration to have a clean buffer
		char buffer[BUFFER_SIZE] = {0};
		// we could do recv non blocking with MSG_DONTWAIT but we will keep it simple for now
		ssize_t bytesRead = recv(_pollFd.fd, buffer, BUFFER_SIZE, 0);
		if (bytesRead > 0)
		{
			_headers.append(buffer, bytesRead);
			_headersTotalBytesRead += bytesRead;
			if (_headersTotalBytesRead > _clientMaxHeadersSize)
			{
				std::cerr << "Header too large" << std::endl;
				_response.setStatusCode(413);
				return false;
			}
			if (_headers.find("\r\n\r\n") != std::string::npos)
				_headersComplete = true;
		}
		else if (bytesRead < 0)
		{
			perror("recv failed");
			return false;
		}
		else
		{
			// This means biyeRead == 0
			std::cout << "Connection closed before headers being completely sent" << std::endl;
			// In this case we don't want to send an error response, because the client closed the connection
			return false;
		}
	}
	return true;
}

bool Connection::readBody()
{
	// Read body from the client socket
	// If the body is complete, set _bodyComplete to true
	// Return true if the body is complete, false otherwise
	return false;
}

bool Connection::isChunked()
{
	// Look for "Transfer-Encoding: chunked" in the headers
	// This would not work cause headers are case insensitive
	// std::string search = "Transfer-Encoding: chunked";
	// return headers.find(search) != std::string::npos;
	std::string lowerHeaders;
	for (std::string::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
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
#include "Client.hpp"

Client::Client(struct pollfd pollFd)
	: _headersComplete(false), _headersTotalBytesRead(0), _bodyComplete(false), _bodyIsChunked(false)
{
	_pollFd.fd = pollFd.fd;
	_pollFd.events = POLLIN | POLLOUT;
	_pollFd.revents = 0;
	memset(&_pollFd, 0, sizeof(_pollFd));
}

Client::Client(const Client &other)
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

Client &Client::operator=(const Client &other)
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

Client::~Client()
{
	// Should I close the file descriptor here?
	if (_pollFd.fd != -1)
	{
		close(_pollFd.fd);
		_pollFd.fd = -1; // Invalidate the file descriptor to prevent double closing
	}
}

bool Client::readHeaders()
{
	// Read headers from the client socket
	// If the headers are complete, set _headersComplete to true
	// If the headers are chunked, set _bodyIsChunked to true
	// If the headers are chunked, read the chunk size and the chunk data
	// If the headers are not chunked, read the content length and the body
	// If the body is complete, set _bodyComplete to true
	// Return true if the headers and body are complete, false otherwise
	return false;
}

bool Client::readBody()
{
	// Read body from the client socket
	// If the body is complete, set _bodyComplete to true
	// Return true if the body is complete, false otherwise
	return false;
}
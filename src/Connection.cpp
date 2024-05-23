#include "Connection.hpp"

Connection::Connection(struct pollfd &pollFd, Server &server)

{
	(void)server;
	_pollFd.fd = pollFd.fd;
	_type = UNDEFINED;
	_serverIp = "";
	_serverPort = 0;
	_pollFd.events = POLLIN;
	_pollFd.revents = 0;
	_hasReadSocket = false;
	_hasFinishedReading = false;
	_hasDataToSend = false;
	_hasFinishedSending = false;
	_canBeClosed = false;
	_responseSize = 0;
	_responseSizeSent = 0;
	_responseString = "";
	_hasServerBlock = NOT_FOUND;
	_hasCGI = false;
	_CGIHasExited = false;
	_CGIPid = 0;
	_CGIStartTime = 0;
	_CGIExitStatus = 0;
	_CGIHasCompleted = false;
	_CGIHasTimedOut = false;
	_CGIHasReadPipe = false;
	_startTime = 0;
}

Connection::Connection(const Connection &other)
{
	_pollFd = other._pollFd;
	_response = other._response;
	_request = other._request;
	_parser = other._parser;
	_type = other._type;
	_serverIp = other._serverIp;
	_serverPort = other._serverPort;
	_request = other._request;
	_parser = other._parser;
	_hasReadSocket = other._hasReadSocket;
	_hasFinishedReading = other._hasFinishedReading;
	_hasDataToSend = other._hasDataToSend;
	_hasFinishedSending = other._hasFinishedSending;
	_canBeClosed = other._canBeClosed;
	_responseSize = other._responseSize;
	_responseSizeSent = other._responseSizeSent;
	_responseString = other._responseString;
	_serverBlock = other._serverBlock;
	_hasServerBlock = other._hasServerBlock;
	_hasCGI = other._hasCGI;
	_CGIHasExited = other._CGIHasExited;
	_CGIPid = other._CGIPid;
	_CGIStartTime = other._CGIStartTime;
	_CGIExitStatus = other._CGIExitStatus;
	_CGIHasCompleted = other._CGIHasCompleted;
	_CGIHasTimedOut = other._CGIHasTimedOut;
	_CGIHasReadPipe = other._CGIHasReadPipe;
	_cgiOutputBuffer = other._cgiOutputBuffer;
	_startTime = other._startTime;
	// std::cout << "Connection object copied" << std::endl;
}

Connection &Connection::operator=(const Connection &other)
{
	if (this != &other)
	{
		_pollFd = other._pollFd;
		_response = other._response;
		_request = other._request;
		_parser = other._parser;
		_type = other._type;
		_serverIp = other._serverIp;
		_serverPort = other._serverPort;
		_hasReadSocket = other._hasReadSocket;
		_hasFinishedReading = other._hasFinishedReading;
		_hasDataToSend = other._hasDataToSend;
		_hasFinishedSending = other._hasFinishedSending;
		_canBeClosed = other._canBeClosed;
		_responseSize = other._responseSize;
		_responseSizeSent = other._responseSizeSent;
		_responseString = other._responseString;
		_serverBlock = other._serverBlock;
		_hasServerBlock = other._hasServerBlock;
		_hasCGI = other._hasCGI;
		_CGIHasExited = other._CGIHasExited;
		_CGIPid = other._CGIPid;
		_CGIStartTime = other._CGIStartTime;
		_CGIExitStatus = other._CGIExitStatus;
		_CGIHasCompleted = other._CGIHasCompleted;
		_CGIHasTimedOut = other._CGIHasTimedOut;
		_CGIHasReadPipe = other._CGIHasReadPipe;
		_cgiOutputBuffer = other._cgiOutputBuffer;
		_startTime = other._startTime;
	}
	std::cout << "Connection object assigned" << std::endl;
	return *this;
}

Connection::~Connection()
{
	// std::cout << "Connection object destroyed" << std::endl;
}

// GETTERS AND SETTERS

// GETTERS

bool Connection::getCGIHasTimedOut() const
{
	return _CGIHasTimedOut;
}

size_t Connection::getResponseSize() const
{
	return _responseSize;
}

size_t Connection::getResponseSizeSent() const
{
	return _responseSizeSent;
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

struct pollfd Connection::getPollFd() const
{
	return _pollFd;
}

struct pollfd &Connection::getPollFd()
{
	return _pollFd;
}

ServerBlock &Connection::getServerBlock()
{
	return _serverBlock;
}

enum ConnectionType Connection::getType() const
{
	return _type;
}

std::string Connection::getResponseString() const
{
	return _responseString;
}

std::string Connection::getServerIp() const
{
	return _serverIp;
}

unsigned short Connection::getServerPort() const
{
	return _serverPort;
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

int Connection::getHasServerBlock() const
{
	return _hasServerBlock;
}

bool Connection::getHasCGI() const
{
	return _hasCGI;
}

pid_t Connection::getCGIPid() const
{
	return _CGIPid;
}

time_t Connection::getCGIStartTime() const
{
	return _CGIStartTime;
}

int Connection::getCGIExitStatus() const
{
	return _CGIExitStatus;
}

bool Connection::getCGIHasCompleted() const
{
	return _CGIHasCompleted;
}

bool Connection::getCGIHasReadPipe() const
{
	return _CGIHasReadPipe;
}

std::string Connection::getCGIOutputBuffer() const
{
	return _cgiOutputBuffer;
}

time_t Connection::getStartTime() const
{
	return _startTime;
}

// SETTERS

void Connection::setStartTime(time_t time)
{
	_startTime = time;
}

void Connection::setCGIOutputBuffer(std::string buffer)
{
	_cgiOutputBuffer = buffer;
}

void Connection::setCGIHasReadPipe(bool value)
{
	_CGIHasReadPipe = value;
}

void Connection::setCGIHasCompleted(bool value)
{
	_CGIHasCompleted = value;
}

void Connection::setResponseSize(size_t responseSize)
{
	_responseSize = responseSize;
}

void Connection::setResponseSizeSent(size_t responseSizeSent)
{
	_responseSizeSent = responseSizeSent;
}

void Connection::setType(enum ConnectionType type)
{
	_type = type;
}

void Connection::setServerIp(std::string serverIp)
{
	_serverIp = serverIp;
}

void Connection::setServerPort(unsigned short serverPort)
{
	_serverPort = serverPort;
}

void Connection::setCGIHasTimedOut(bool value)
{
	_CGIHasTimedOut = value;
}

void Connection::setHasReadSocket(bool value)
{
	_hasReadSocket = value;
}

void Connection::setHasFinishedReading(bool value)
{
	_hasFinishedReading = value;
}

void Connection::setHasDataToSend(bool value)
{
	if (value == true)
		_pollFd.events = POLLOUT;
	_hasDataToSend = value;
}

void Connection::setResponseString(std::string responseString)
{
	_responseString = responseString;
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

void Connection::setHasCGI(bool value)
{
	_hasCGI = value;
}

void Connection::setCGIPid(pid_t pid)
{
	_CGIPid = pid;
}

void Connection::setCGIStartTime(time_t time)
{
	_CGIStartTime = time;
}

void Connection::setCGIExitStatus(int status)
{
	_CGIExitStatus = status;
}

// METHODS

bool Connection::readHeaders(Parser &parser)
{
	// std::cout << "\nEntering readHeaders" << std::endl;
	const int bufferSize = BUFFER_SIZE;
	char buffer[bufferSize] = {0};
	std::cout << "buffers size: " << sizeof(buffer) << std::endl;
	ssize_t bytesRead = recv(_pollFd.fd, buffer, bufferSize, 0);
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
		// TODO: think about it
		std::cout << "Connection closed before headers being completely sent" << std::endl;
		return false;
		// std::cout << "bytes_read == 0" << std::endl;
		// return true;
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
			return true;
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

/* CGI */
void Connection::addCGI(pid_t pid)
{
	_hasCGI = true;
	_CGIPid = pid;
	std::cout << "CGI process added with pid: " << _CGIPid << std::endl;
	_CGIStartTime = time(NULL);
}

void Connection::removeCGI(int status)
{
	// _hasCGI = false;
	// maybe better to set it to false after we have finished sending the response
	_CGIHasExited = true;
	_CGIPid = 0;
	_CGIStartTime = 0;
	_CGIExitStatus = status;
}

/* Debugging */

void Connection::printConnection() const
{
	std::cout << CYAN << "CONNECTION # " << _pollFd.fd << " (fd)" << RESET << std::endl;

	std::string connectionType;

	if (_type == SERVER)
		connectionType = "SERVER";
	else if (_type == CLIENT)
		connectionType = "CLIENT";
	else
		connectionType = "UNDEFINED";

	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "type:" << connectionType << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasReadSocket:" << _hasReadSocket << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasFinishedReading:" << _hasFinishedReading
			  << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasDataToSend:" << _hasDataToSend << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "hasFinishedSending:" << _hasFinishedSending
			  << std::endl;
	std::cout << std::setw(5) << ' ' << std::left << std::setw(22) << "canBeClosed:" << _canBeClosed << std::endl;
}

bool Connection::findServerBlock(const std::vector<ServerBlock> &serverBlocks)
{
	std::string serverName;

	for (size_t i = 0; i < serverBlocks.size(); i++)
	{
		// loop through the server names
		for (size_t j = 0; j < serverBlocks[i].getServerName().size(); j++)
		{
			serverName = serverBlocks[i].getServerName()[j];
			if (serverName == _request.getSingleHeader("host").second)
				break;
		}

		if (serverName == _request.getSingleHeader("host").second)
		{
			// loop through the listen entries
			for (size_t k = 0; k < serverBlocks[i].getListen().size(); k++)
			{
				if (serverBlocks[i].getListen()[k].getPort() == _serverPort &&
					serverBlocks[i].getListen()[k].getIp() == _serverIp)
				{
					_serverBlock = serverBlocks[i];

					if (_request.getMethod() == "POST" && _serverBlock.getClientMaxBodySize() != 0 &&
						_request.getContentLength() > _serverBlock.getClientMaxBodySize())
						_response.setStatusCode(413, "Payload Too Large");

					_hasServerBlock = FOUND;
					return (true);
				}
			}
		}
	}

	// check if default server block exists for error_pages
	for (size_t i = 0; i < serverBlocks.size(); i++)
	{
		// loop through the listen entries
		for (size_t k = 0; k < serverBlocks[i].getListen().size(); k++)
		{
			if (serverBlocks[i].getListen()[k].getPort() == _serverPort &&
				serverBlocks[i].getListen()[k].getIp() == _serverIp)
			{
				if (_request.getMethod() == "POST" && _serverBlock.getClientMaxBodySize() != 0 &&
					_request.getContentLength() > _serverBlock.getClientMaxBodySize())
				{
					_response.setStatusCode(413, "Payload Too Large");
					return false;
				}
				Debug::log("Default server block found", Debug::NORMAL);
				_hasServerBlock = DEFAULT;
				_response.setStatusCode(404, "Not Found");
				return (_serverBlock = serverBlocks[i], true);
			}
		}
	}

	Debug::log("Server block not found", Debug::NORMAL);
	_response.setStatusCode(404, "Not Found");
	return (false);
}

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
	Debug::log("Connection object assigned", Debug::OCF);
	return *this;
}

Connection::~Connection()
{
	Debug::log("Connection object destroyed", Debug::OCF);
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

ssize_t Connection::readSocket(char *buffer, size_t bufferSize)
{
	ssize_t bytesRead = recv(_pollFd.fd, buffer, bufferSize, 0);
	if (bytesRead < 0)
	{
		perror("recv failed");
		return -1;
	}
	return bytesRead;
}

bool Connection::readHeaders(Parser &parser)
{
	const int bufferSize = BUFFER_SIZE;
	char buffer[bufferSize] = {0};
	Debug::log("buffer size: " + toString(sizeof(buffer)), Debug::SERVER);
	ssize_t bytesRead = readSocket(buffer, bufferSize);
	Debug::log("bytesRead: " + toString(bytesRead), Debug::SERVER);
	if (bytesRead > 0)
	{
		parser.setBuffer(parser.getBuffer() + std::string(buffer, bytesRead));
		return true;
	}
	else if (bytesRead < 0)
	{
		// TODO: All these errors should be always logged
		Debug::log("Error on recv, possibly a system error", Debug::SERVER);
		perror("recv failed");
		return false;
	}
	else if (bytesRead == 0)
	{
		Debug::log("Connection gracefully closed by peer before headers being completely sent", Debug::SERVER);
		perror("recv failed");
		return false;
	}
	else
	{
		Debug::log("Exiting readHeaders. This will never happen here!", Debug::SERVER);
		return true;
	}
}

bool Connection::readChunkedBody(Parser &parser)
{
	if (!parser.getBodyComplete())
	{
		std::string chunkSizeLine;
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
	line.clear();
	// TODO: this is actually a possible infinite loop!
	while (true)
	{
		char buffer = 0;
		ssize_t bytesRead = readSocket(&buffer, 1);
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
			Debug::log("Error on recv, possibly a system error", Debug::SERVER);
			perror("recv failed");
			return false;
		}
		else if (bytesRead == 0)
		{
			Debug::log("Connection closed gracefully by the peer while reading the chunk size of a chunk",
					   Debug::SERVER);
			return false;
		}
	}
	return true; // Unreachable
}

bool Connection::readChunk(size_t chunkSize, std::string &chunkData, HTTPResponse &response)
{
	// TODO: think if we need to set the error here and not in handlePOstAndDelete
	// Reserve space in the string to avoid reallocations
	chunkData.reserve(chunkSize + chunkData.size());
	while (chunkSize > 0)
	{
		char buffer[BUFFER_SIZE];
		size_t bytesToRead = std::min(chunkSize, (size_t)BUFFER_SIZE);
		ssize_t bytesRead = readSocket(buffer, bytesToRead);
		if (bytesRead > 0)
		{
			chunkData.append(buffer, bytesRead);
			chunkSize -= bytesRead;
		}
		else if (bytesRead < 0)
		{
			Debug::log("Error on recv, possibly a system error", Debug::SERVER);
			perror("recv failed in readChunk");
			response.setStatusCode(500, "");
			return false;
		}
		else if (bytesRead == 0)
		{
			Debug::log("Connection closed before headers being completely sent", Debug::SERVER);
			response.setStatusCode(400, "Connection closed while reading chunk"); // Bad Request
			return false;
		}
	}
	// TODO: check if we need this
	char crlf[2];
	ssize_t crlfRead = readSocket(crlf, 2);
	if (crlfRead < 2)
	{
		response.setStatusCode(400, "Connection closed while reading CRLF"); // Bad Request
		return false;
	}
	return true;
}

bool Connection::readBody(Parser &parser, HTTPRequest &req, HTTPResponse &res)
{
	size_t contentLength = req.getContentLength();

	char buffer[BUFFER_SIZE];
	// We could also use _bodyTotalBytesRead from the parser
	size_t bytesRead = parser.getBuffer().size();
	Debug::log("contentLength: " + toString(contentLength), Debug::SERVER);
	Debug::log("bytesRead: " + toString(bytesRead), Debug::SERVER);
	if (bytesRead < contentLength)
	{
		ssize_t read = readSocket(buffer, BUFFER_SIZE);
		if (read > 0)
		{
			parser.setBuffer(parser.getBuffer() + std::string(buffer, read));
			Debug::log("bytesRead: " + toString(parser.getBuffer().size()), Debug::SERVER);
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
		else if (bytesRead == 0)
		{
			Debug::log("Connection closed before body being completely sent", Debug::SERVER);
			res.setStatusCode(499, "Connection closed by the client"); // Client Closed Request
			return false;
			// return true;
		}
	}
	else
		parser.setBodyComplete(true);
	return true;
}

/* CGI */
void Connection::addCGI(pid_t pid)
{
	_hasCGI = true;
	_CGIPid = pid;
	Debug::log("CGI process added with pid: " + toString(_CGIPid), Debug::CGI);
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

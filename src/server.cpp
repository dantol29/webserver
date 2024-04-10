#include "Server.hpp"

// Default constructor
Server::Server()
{
	loadDefaultConfig();
}
// Constructor with config file path
Server::Server(const std::string configFilePath) : _configFilePath(configFilePath)
{
	loadDefaultConfig();
	loadConfig();
}
// Destructor
Server::~Server()
{
}

// Start listening
void Server::startListening()
{

	createServerSocket();
	setReuseAddrAndPort();
	bindToPort(_port);
	listen();
}

void Server::startPollEventLoop()
{
	addServerSocketPollFdToFDs();
	while (1)
	{
		std::cout << "++++++++++++++ Waiting for new connection +++++++++++++++" << std::endl;
		int ret = poll(_FDs.data(), _FDs.size(), -1);
		if (ret > 0)
		{
			for (size_t i = 0; i < _FDs.size(); i++)
			{
				if (_FDs[i].revents & POLLIN)
				{
					if (i == 0)
						acceptNewConnection();
					else
					{
						// TODO: only the index is actually needed
						handleConnection(_FDs[i].fd);
						_FDs.erase(_FDs.begin() + i);
						--i;
					}
				}
				else if (_FDs[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					if (i == 0)
						handleServerSocketError();
					else
						handleClientSocketError(_FDs[i].fd, i);
				}
			}
		}
		else if (ret == 0)
			handleSocketTimeoutIfAny();
		else
			handlePollFailure();
	}
}

void Server::handleConnection(int clientFD)
{
	std::string headers;
	HTTPResponse response;
	if (!readHeaders(clientFD, headers, response))
	{
		closeClientConnection(clientFD, response);
		return;
	}
	std::string body;
	if (isChunked(headers))
	{
		if (!readChunkedBody(clientFD, body, response))
		{
			closeClientConnection(clientFD, response);
			return;
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
			ssize_t read = recv(clientFD, buffer, BUFFER_SIZE, 0);
			if (read > 0)
			{
				body.append(buffer, read);
				bytesRead += read;
			}
			else if (read < 0)
			{
				perror("recv failed");
				// Consiger if we should close the clientFD or retry the recv
				close(clientFD);
				return;
			}
			else
			{
				std::cout << "Connection closed" << std::endl;
				close(clientFD);
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

	// std::string response;
	Router router;
	if (!router.pathExists(response, obj.getRequestTarget()))
	{
		StaticContentHandler staticContentHandler;
		// This shoud be a method of the requestHandler obect
		// response = router.handleNotFound();
		response = staticContentHandler.handleNotFound();
	}
	else if (router.isDynamicRequest(obj))
	{
		if (obj.getMethod() == "GET" && obj.getRequestTarget() == "/hello")
		{
			// env has to be created before CGI, because it is passed to the CGI
			CGIHandler cgiHandler;
			Environment env;
			env.setVar("QUERY_STRING", "Hello from C++ CGI!");
			// cgiHandler.executeCGI(argv, env);
			handleCGIRequest(argv, env);
		}
		else
		{
			CGIHandler cgiHandler;
			Environment env;
			env.setVar("obj.getQueryString()", "obj.getBody()");
			// response = cgiHandler.handleCGIRequest(argv, obj);
			// cgiHandler.executeCGI(argv, env);
			handleCGIRequest(argv, env);
		}
	}
	else
	{
		StaticContentHandler staticContentHandler;
		// This if condition only for legacy reasons! TODO: remove
		if (obj.getMethod() == "GET" && (obj.getRequestTarget() == "/" || obj.getRequestTarget() == "/home"))
		{
			response = staticContentHandler.handleHomePage();
		}
		else
		{
			response = staticContentHandler.handleRequest(obj);
		}
	}
	std::string responseString = response.toString();

	write(clientFD, responseString.c_str(), responseString.size());
	close(clientFD);
}

/*** Private Methods ***/

void Server::loadConfig()
{
	// Add logic to load config from file
}

void Server::loadDefaultConfig()
{
	_webRoot = "var/www";
	_maxClients = 10;
}

/* startListening */

void Server::createServerSocket()
{
	if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		perrorAndExit("Failed to create server socket");
}

void Server::setReuseAddrAndPort()
{
	int opt = 1;
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEADDR: Protocol not available, continuing without SO_REUSEADDR");
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
}

void Server::bindToPort(int port)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_serverAddr.sin_port = htons(port);
	std::memset(_serverAddr.sin_zero, '\0', sizeof _serverAddr.sin_zero);

	if (bind(_serverFD, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		perrorAndExit("In bind");
}

void Server::listen()
{
	if (::listen(_serverFD, _maxClients) < 0)
		perrorAndExit("In listen");
}

/* startPollEventsLoop */

void Server::addServerSocketPollFdToFDs()
{
	struct pollfd serverPollFd;
	serverPollFd.fd = _serverFD;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_FDs.push_back(serverPollFd);
}

void Server::acceptNewConnection()
{
	struct sockaddr_in clientAddress;
	socklen_t ClientAddrLen = sizeof(clientAddress);
	std::cout << "New connection detected" << std::endl;
	int newSocket = accept(_serverFD, (struct sockaddr *)&clientAddress, (socklen_t *)&ClientAddrLen);
	if (newSocket >= 0)
	{
		struct pollfd newSocketPoll;
		newSocketPoll.fd = newSocket;
		newSocketPoll.events = POLLIN;
		newSocketPoll.revents = 0;
		_FDs.push_back(newSocketPoll);
		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);
		std::cout << "New connection from " << clientIP << std::endl;
	}
	else
	{
		// TODO: consider what to do here. Not sure we want to exit the program.
		perror("In accept");
	}
}

void Server::handleServerSocketError()
{
	perror("poll server socket error");
}

void Server::handleClientSocketError(int clientFD, size_t &i)
{
	close(clientFD);
	_FDs.erase(_FDs.begin() + i);
	perror("poll client socket error");
	--i;
}

void Server::handleSocketTimeoutIfAny()
{
	// Is not the socket timeout, but the poll timeout
	std::cout << "Timeout occurred!" << std::endl;
	// This should never happen with an infinite timeout
}

void Server::handlePollFailure()
{
	// linear issue: https://linear.app/web-serv/issue/WEB-91/implement-adequate-response-on-poll-failure
	if (errno == EINTR)
	{
		// The call was interrupted by a signal. Log and possibly retry.
		std::cerr << "poll() interrupted by signal. Retrying." << std::endl;
		// Basically we are not doing anything here.
	}
	else
	{
		// Log the error with as much detail as available.
		perror("Critical poll error from handlePollFailure()");

		// EBADF, EFAULT, EINVAL, ENOMEM indicate more severe issues.
		// Depending on the nature of your server, you might try to clean up and restart polling,
		// or log the failure and exit for manual recovery.
		// For a server, exiting might not be the best choice without trying to recover,
		// but ensure admins are alerted for investigation.

		// Example: Send alert to admin or trigger automatic recovery process
		AlertAdminAndTryToRecover();

		// In critical cases, consider a graceful shutdown.
		// exit(EXIT_FAILURE); or better yet, GracefulShutdown();
	}
}

void Server::AlertAdminAndTryToRecover()
{
	std::cerr << "Calling Leo and Daniil to fix the server" << std::endl;
	std::cerr << "Super dope function that recovers the server" << std::endl;
}

/* for handleConnection */

void Server::closeClientConnection(int clientFD, HTTPResponse &response)
{
	if (response.getStatusCode() != 0)
	{
		std::string responseString = response.toString();
		send(clientFD, responseString.c_str(), responseString.size(), 0);
	}
	close(clientFD);
}

bool Server::readHeaders(int clientFD, std::string &headers, HTTPResponse &response)
{
	size_t totalRead = 0;
	bool headersComplete = false;
	while (!headersComplete)
	{
		// We reinitialize it at each iteration to have a clean buffer
		char buffer[BUFFER_SIZE] = {0};
		// we could do recv non blocking with MSG_DONTWAIT but we will keep it simple for now
		ssize_t bytesRead = recv(clientFD, buffer, BUFFER_SIZE, 0);
		if (bytesRead > 0)
		{

			headers.append(buffer, bytesRead);
			totalRead += bytesRead;
			if (totalRead > MAX_HEADER_SIZE)
			{
				std::cerr << "Header too large" << std::endl;
				response.setStatusCode(413);
				return false;
			}
			if (headers.find("\r\n\r\n") != std::string::npos)
				headersComplete = true;
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

bool Server::readChunkedBody(int clientFd, std::string &body, HTTPResponse &response)
{
	// TODO: check if this is blocking; I mean the two recvs in readChunkSize and readChunk
	bool bodyComplete = false;
	while (!bodyComplete)
	{
		std::string chunkSizeLine;
		if (!readChunkSize(clientFd, chunkSizeLine))
			return false;
		size_t chunkSize = std::stoul(chunkSizeLine, 0, 16);
		if (chunkSize == 0)
		{
			bodyComplete = true;
			return true;
		}
		else
		{
			std::string chunkData;
			// readChunk(clientFd, chunkSize, chunkData, response);
			if (!readChunk(clientFd, chunkSize, chunkData, response))
			{
				closeClientConnection(clientFd, response);
				return false;
			}
			body.append(chunkData);
			// Consume the CRLF at the end of the chunk
		}
	}
	return false;
}

/* Others */

std::string Server::getWebRoot() const
{
	return _webRoot;
}

void Server::setWebRoot(const std::string &webRoot)
{
	_webRoot = webRoot;
}

std::string Server::getConfigFilePath() const
{
	return _configFilePath;
}

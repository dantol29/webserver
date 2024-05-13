#include "Server.hpp"
#include "Parser.hpp"
#include "Connection.hpp"

#define SEND_BUFFER_SIZE 1024 * 100 // 100 KB

Server::Server()
{
	loadDefaultConfig();
}

Server::Server(const Config &config)
{
	_config = config;
	// while we don't have a config file
	loadDefaultConfig();
}

Server::~Server()
{
}

void Server::startListening()
{

	createServerSocket();
	setReuseAddrAndPort();
	checkSocketOptions();
	bindToPort(_port);
	listen();
}

void Server::startPollEventLoop()
{
	addServerSocketPollFdToVectors();
	int pollCounter = 0;
	while (1)
	{
		// printConnections("BEFORE POLL", _FDs, _connections, true);
		std::cout << CYAN << "++++++++++++++ #" << pollCounter
				  << " Waiting for new connection or Polling +++++++++++++++" << RESET << std::endl;
		int ret = poll(_FDs.data(), _FDs.size(), -1);
		pollCounter++;
		// printFrame("POLL EVENT DETECTED", true);
		// printConnections("AFTER POLL", _FDs, _connections, true);
		if (ret > 0)
		{
			size_t originalSize = _FDs.size();
			// if _FDs becomes bigger than originalSize we don't want to loop over the new elements before we finish the
			// old ones if _FDs becomes smaller than originalSize we don't want to loop over the old elements that are
			// not in _FDs anymore
			for (size_t i = 0; i < originalSize && i < _FDs.size(); i++)
			{
				if (_FDs[i].revents & (POLLIN | POLLOUT))
				{
					Debug::log("Enters revents", Debug::OCF);
					if (i == 0)
					{
						// printFrame("SERVER SOCKET EVENT", true);
						acceptNewConnection(_connections[i]);
					}
					else
					{
						// printFrame("CLIENT SOCKET EVENT", true);
						handleConnection(_connections[i],
										 i,
										 _connections[i].getParser(),
										 _connections[i].getRequest(),
										 _connections[i].getResponse());
						if (_connections[i].getHasFinishedReading() && _connections[i].getHasDataToSend())
							_FDs[i].events = POLLOUT;
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
			handlePollError();
	}
}

void Server::readFromClient(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	(void)i;
	Debug::log("\033[1;33mEntering read from client\033[0m", Debug::OCF);
	// TODO: change to _areHeadersCopmplete
	if (!parser.getHeadersComplete())
	{
		Debug::log("\033[1;33mReading headers\033[0m", Debug::NORMAL);
		if (!conn.readHeaders(parser))
		{
			Debug::log("Error reading headers", Debug::OCF);
			conn.setHasFinishedReading(true);
			conn.setHasDataToSend(false);
			conn.setCanBeClosed(true);
			return;
		}
		conn.setHasReadSocket(true);
		if (!parser.preParseHeaders(response))
		{
			// logic was incorrect here
			conn.setCanBeClosed(false);
			conn.setHasFinishedReading(true);
			conn.setHasDataToSend(false);
			// ---------------------
			std::cout << "Error pre-parsing headers" << std::endl;
			return;
		}
	}
	if (!parser.getHeadersComplete())
	{
		Debug::log("Headers incomplete yet, exiting readFromClient.", Debug::NORMAL);
		return;
	}
	if (parser.getHeadersComplete() && !parser.getHeadersAreParsed())
		parser.parseRequestLineAndHeaders(parser.getHeadersBuffer().c_str(), request, response);
	if (response.getStatusCode() != 0)
	{
		conn.setCanBeClosed(false);
		conn.setHasFinishedReading(true);
		conn.setHasDataToSend(false);
		Debug::log("Error parsing headers or request line", Debug::OCF);
		return;
	}
	std::cout << parser.getHeadersComplete() << " ," << request.getMethod() << std::endl;
	if (parser.getHeadersComplete() && request.getMethod() == "GET")
		conn.setHasFinishedReading(true);

	if (response.getStatusCode() != 0)
		Debug::log(toString(response.getStatusCode()), Debug::NORMAL);
	if (request.getMethod() == "GET")
		Debug::log("GET request, no body to read", Debug::NORMAL);
	else
	{
		if (parser.getIsChunked() && !conn.getHasReadSocket())
		{
			Debug::log("Chunked body", Debug::NORMAL);
			if (!conn.readChunkedBody(parser))
			{
				// Case of error while reading chunked body
				Debug::log("Error reading chunked body", Debug::OCF);
				conn.setCanBeClosed(true);
				conn.setHasFinishedReading(true);
				// It could be that we had data that could be sent even if we have an error cause previous data was read
				return;
			}
			conn.setHasReadSocket(true);
		}
		else if (!conn.getHasReadSocket())
		{
			Debug::log("\033[1;33mReading body\033[0m", Debug::NORMAL);
			// TODO: add comments
			if (!parser.getBodyComplete() && parser.getBuffer().size() == request.getContentLength())
			{
				// TODO: in the new design we will return here and go to the function where the response is
				parser.setBodyComplete(true);
				conn.setHasFinishedReading(true);
				conn.setHasDataToSend(true);
			}
			else if (!conn.getHasReadSocket() && !conn.readBody(parser, request, response, _config))
			{
				Debug::log("Error reading body", Debug::OCF);
				conn.setCanBeClosed(false);
				conn.setHasFinishedReading(true);
				conn.setHasDataToSend(false);
				return;
			}
		}
		if (!parser.getBodyComplete() && request.getContentLength() != 0 &&
			parser.getBuffer().size() == request.getContentLength())
		{
			// TODO: in the new design we will return here and go to the function where the response is
			parser.setBodyComplete(true);
			conn.setHasFinishedReading(true);
			conn.setCanBeClosed(false);
			conn.setHasDataToSend(false);
			return;
		}
		if (!parser.getBodyComplete())
		{
			Debug::log("Body still incomplete, exiting readFromClient.", Debug::NORMAL);
			conn.setHasFinishedReading(false);
			conn.setHasReadSocket(true);
			return;
		}
		//  std::cout << parser.getBuffer() << std::endl;
		if (!request.getUploadBoundary().empty())
			parser.parseFileUpload(parser.getBuffer(), request, response);
		else if (request.getMethod() != "GET")
		{
			request.setBody(parser.getBuffer());
			conn.setHasFinishedReading(true);
		}
	}
}

void Server::buildResponse(Connection &conn, size_t &i, HTTPRequest &request, HTTPResponse &response)
{
	(void)i;
	Debug::log("Entering buildResponse", Debug::NORMAL);
	Debug::log("Request method: " + request.getMethod(), Debug::NORMAL);

	ServerBlock serverBlock;
	Directives directive;
	std::string serverName;
	std::cout << GREEN << "Number of server blocks: " << _config.getServerBlocks().size() << RESET << std::endl;
	std::cout << "Request host: " << request.getSingleHeader("host").second << std::endl;
	std::cout << "Request target: " << request.getRequestTarget() << std::endl;

	// if there is "?" in the request target, we need to remove it
	if (std::find(request.getRequestTarget().begin(), request.getRequestTarget().end(), '?') != request.getRequestTarget().end())
		request.setRequestTarget(request.getRequestTarget().substr(0, request.getRequestTarget().find("?")));
	std::cout << "Request target: " << request.getRequestTarget() << std::endl;

	for (size_t i = 0; i < _config.getServerBlocks().size(); i++)
	{
		// loop through all server names in the server block
		for (size_t j = 0; j < _config.getServerBlocks()[i].getServerName().size(); j++)
		{
			serverName = _config.getServerBlocks()[i].getServerName()[j];
			std::cout << RED << "Checking server name: " << serverName << RESET << std::endl;
			if (serverName == request.getSingleHeader("host").second){
				std::cout << GREEN << "Server name found" << RESET << std::endl;
				break ;
			}
		}
		if (serverName == request.getSingleHeader("host").second)
		{
			// _config.setServerBlockIndex(i);
			serverBlock = _config.getServerBlocks()[i];
			directive = serverBlock.getDirectives();
			std::cout << "Request target in block: " << request.getRequestTarget() << std::endl;
			
			for (size_t i = 0; i < serverBlock.getLocations().size(); i++)
			{
				std::cout << "Location: " << serverBlock.getLocations()[i]._path << " == " << request.getRequestTarget() << std::endl;
				if (request.getRequestTarget() == serverBlock.getLocations()[i]._path)
				{
					std::cout << "Location found" << std::endl;
					directive = serverBlock.getLocations()[i];
					break;
				}
			}
			break;
		}
		else if (i == _config.getServerBlocks().size() - 1)
		{
			static StaticContentHandler staticContentInstance;
			// if error already occurred, we don't want to overwrite it
			if (response.getStatusCode() != 0)
			{
				Debug::log("Error response" + toString(response.getStatusCode()), Debug::NORMAL);
				response.setErrorResponse(response.getStatusCode());
				conn.setHasDataToSend(true);
				return;
			}
			// if no server name is found, use the default server block
			staticContentInstance.handleNotFound(response);
			response.setStatusCode(404, "No server block is matching the request host");
			conn.setHasDataToSend(true);
			Debug::log("Exiting buildResponse", Debug::NORMAL);
			return;
		}
		std::cout << "Index: " << i << std::endl;
	}

	std::string root = directive._root;

	std::cout << "Root: " << root << std::endl;
	if (root[root.size() - 1] != '/')
		root = root + "/";
	std::cout << RED << "Root: " << root << RESET << std::endl;

	Router router(directive);

	if (response.getStatusCode() != 0)
	{
		Debug::log("Error response" + toString(response.getStatusCode()), Debug::NORMAL);
		// response.setErrorResponse(response.getStatusCode());
		router.handleServerBlockError(request, response, response.getStatusCode());
		conn.setHasDataToSend(true);
		return;
	}
	else
	{
		router.setFDsRef(&_FDs);
		router.setPollFd(&conn.getPollFd());
		router.routeRequest(request, response);
	}
	// TODO: check if the listen in the server block is matching port and ip from connection
	conn.setHasDataToSend(true);
}

void Server::writeToClient(Connection &conn, size_t &i, HTTPResponse &response)
{
	std::cout << "\033[1;36m" << "Entering writeToClient" << "\033[0m" << std::endl;
	std::cout << response << std::endl;
	static int sendResponseCounter = 0;
	bool isLastSend = false;
	size_t tmpBufferSize = SEND_BUFFER_SIZE;
	(void)i;

	if (conn.getResponseSizeSent() == 0)
	{
		conn.setResponseString(response.objToString());
		conn.setResponseSize(response.objToString().size());
		sendResponseCounter = 0;
	}

	if (conn.getResponseString().size() < SEND_BUFFER_SIZE)
	{
		tmpBufferSize = conn.getResponseString().size();
		std::cout << GREEN << "Sending last part of the response" << RESET << std::endl;
		isLastSend = true;
	}

	std::cout << GREEN << "sendResponseCounter: " << sendResponseCounter << RESET << std::endl;
	int read = send(conn.getPollFd().fd, conn.getResponseString().c_str(), tmpBufferSize, 0);
	if (read == -1)
	{
		perror("send");
	}

	sendResponseCounter++;
	conn.setResponseSizeSent(conn.getResponseSizeSent() + tmpBufferSize);
	if (isLastSend)
	{
		conn.setHasFinishedSending(true);
		conn.setCanBeClosed(true);
	}

	conn.setResponseString(conn.getResponseString().substr(tmpBufferSize));
	response.getBody().erase(0, SEND_BUFFER_SIZE);
}

void Server::closeClientConnection(Connection &conn, size_t &i)
{
	std::cout << "\033[1;36m" << "Entering closeClientConnection" << "\033[0m" << std::endl;
	// TODO: should we close it with the Destructor of the Connection class?
	close(conn.getPollFd().fd);
	_FDs.erase(_FDs.begin() + i);
	_connections.erase(_connections.begin() + i);
	--i;
}

void Server::handleConnection(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	std::cout << "\033[1;36m" << "Entering handleConnection" << "\033[0m" << std::endl;

	conn.setHasReadSocket(false);
	std::cout << "Has finished reading: " << conn.getHasFinishedReading() << std::endl;
	if (!conn.getHasFinishedReading())
		readFromClient(conn, i, parser, request, response);

	if (conn.getHasReadSocket() && !conn.getHasFinishedReading())
	{
		std::cout << "Has read socket: " << conn.getHasReadSocket() << std::endl;
		return;
	}
	std::cout << request << std::endl;
	if (!conn.getCanBeClosed() && !conn.getHasDataToSend())
		buildResponse(conn, i, request, response);

	if (conn.getHasDataToSend() && !conn.getHasReadSocket())
		writeToClient(conn, i, response);

	if (conn.getCanBeClosed())
		closeClientConnection(conn, i);
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
	_clientMaxHeadersSize = CLIENT_MAX_HEADERS_SIZE;
	_clientMaxBodySize = CLIENT_MAX_BODY_SIZE;
	_port = 8080;
}

/* startListening */

void Server::createServerSocket()
{
	if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		perrorAndExit("Failed to create server socket");
	// std::cout << "Server socket created" << std::endl;
	// std::cout << "Server socket file descriptor: " << _serverFD << std::endl;
}

void Server::setReuseAddrAndPort()
{
	int opt = 1;
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEADDR: Protocol not available, continuing without SO_REUSEADDR");
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
	// std::cout << "SO_REUSEADDR and SO_REUSEPORT set" << std::endl;
}

void Server::bindToPort(int port)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_serverAddr.sin_port = htons(port);
	std::memset(_serverAddr.sin_zero, '\0', sizeof _serverAddr.sin_zero);

	if (bind(_serverFD, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		perrorAndExit("In bind");
	// std::cout << "Server socket bound to port " << port << std::endl;
	// std::cout << "Server socket address: " << inet_ntoa(_serverAddr.sin_addr) << std::endl;
	// std::cout << "Server socket port: " << ntohs(_serverAddr.sin_port) << std::endl;
}

void Server::listen()
{
	if (::listen(_serverFD, _maxClients) < 0)
		perrorAndExit("In listen");
	std::cout << "Server socket listening on port " << ntohs(_serverAddr.sin_port) << std::endl;
}

/* startPollEventsLoop */

void Server::addServerSocketPollFdToVectors()
{
	// In this function we also create the struct pollfd for the server socket
	struct pollfd serverPollFd;
	memset(&serverPollFd, 0, sizeof(serverPollFd));
	serverPollFd.fd = _serverFD;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	if (VERBOSE)
	{
		std::cout << "pollfd struct for Server socket created" << std::endl;
		std::cout << std::endl;
		std::cout << "Printing serverPollFd (struct pollfd) before push_back into _FDs" << std::endl;
		std::cout << "fd: " << serverPollFd.fd << ", events: " << serverPollFd.events
				  << ", revents: " << serverPollFd.revents << std::endl;
	}
	_FDs.push_back(serverPollFd);
	Connection serverConnection(serverPollFd, *this);
	serverConnection.setType(SERVER);
	serverConnection.setServerIp(inet_ntoa(_serverAddr.sin_addr));
	serverConnection.setServerPort(ntohs(_serverAddr.sin_port));
	if (VERBOSE)
	{
		std::cout << "Server Connection object created" << std::endl;
		std::cout << MAGENTA << "Printing serverConnection before push_back" << std::endl << RESET;
		serverConnection.printConnection();
	}
	_connections.push_back(serverConnection);
	if (VERBOSE)
	{
		std::cout << MAGENTA << "Printing serverConnection after push_back" << RESET << std::endl;
		_connections.back().printConnection();
		std::cout << "Server socket pollfd added to vectors" << std::endl;
	}
}

void Server::acceptNewConnection(Connection &conn)
{
	// TODO: think about naming.
	// We have 4 different names for kind of the same thing: clientAddress, newSocket, newSocketPoll,
	// newConnection
	struct sockaddr_in clientAddress;
	socklen_t ClientAddrLen = sizeof(clientAddress);
	std::cout << "New connection detected" << std::endl;
	// int newSocket = accept(_serverFD, (struct sockaddr *)&clientAddress, (socklen_t *)&ClientAddrLen);
	int newSocket = accept(conn.getPollFd().fd, (struct sockaddr *)&clientAddress, (socklen_t *)&ClientAddrLen);
	if (newSocket >= 0)
	{
		struct pollfd newSocketPoll;
		newSocketPoll.fd = newSocket;
		newSocketPoll.events = POLLIN;
		newSocketPoll.revents = 0;
		Connection newConnection(newSocketPoll, *this);
		newConnection.setType(CLIENT);
		newConnection.setServerIp(conn.getServerIp());
		newConnection.setServerPort(conn.getServerPort());
		if (VERBOSE)
		{

			std::cout << PURPLE << "BEFORE PUSH_BACK" << RESET << std::endl;
			std::cout << "Printing newConnection:" << std::endl;
			newConnection.printConnection();
			std::cout << "Printing struct pollfd newSocketPoll:" << std::endl;
			std::cout << "fd: " << newSocketPoll.fd << ", events: " << newSocketPoll.events
					  << ", revents: " << newSocketPoll.revents << std::endl;
		}
		/* start together */
		_FDs.push_back(newSocketPoll);
		_connections.push_back(newConnection);
		std::cout << newConnection.getHasFinishedReading() << std::endl;
		std::cout << _connections.back().getHasFinishedReading() << std::endl;
		/* end together */
		if (VERBOSE)
		{
			std::cout << PURPLE << "AFTER PUSH_BACK" << RESET << std::endl;
			std::cout << "Printing last element of _FDs:" << std::endl;
			std::cout << "fd: " << _FDs.back().fd << ", events: " << _FDs.back().events
					  << ", revents: " << _FDs.back().revents << std::endl;
			std::cout << "Printing last element of _connections:" << std::endl;
			_connections.back().printConnection();
			std::cout << "Pringing the whole _FDs and _connections vectors after adding new connection" << std::endl;
			print_connectionsVector(_connections);
			printFDsVector(_FDs);
		}
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
	static int errorCounter = 0;
	perror("poll server socket error");
	if (errorCounter > 5)
	{
		std::cerr << "Too many errors on server socket. Exiting." << std::endl;
		exit(EXIT_FAILURE);
	}
	++errorCounter;
}

void Server::handleClientSocketError(int clientFD, size_t &i)
{
	std::cout << "handleClientSocketError" << std::endl;
	close(clientFD);
	/* start together */
	_FDs.erase(_FDs.begin() + i);
	_connections.erase(_connections.begin() + i);
	/* end together */
	--i;
	perror("poll client socket error");
}

void Server::handleSocketTimeoutIfAny()
{
	// Is not the socket timeout, but the poll timeout
	std::cout << "Timeout occurred!" << std::endl;
	// This should never happen with an infinite timeout
}

void Server::handlePollError()
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
		perror("Critical poll error from handlePollError()");

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

/* Others */

size_t Server::getClientMaxHeadersSize() const
{
	return _clientMaxHeadersSize;
}

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

void Server::checkSocketOptions()
{
	int optval;
	socklen_t optlen = sizeof(optval);

	if (getsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen) < 0)
	{
		perror("getsockopt SO_REUSEADDR failed");
	}
	else
	{
		// std::cout << "SO_REUSEADDR is " << (optval ? "enabled" : "disabled") << std::endl;
	}

	if (getsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen) < 0)
	{
		perror("getsockopt SO_REUSEPORT failed");
	}
	else
	{
		// std::cout << "SO_REUSEPORT is " << (optval ? "enabled" : "disabled") << std::endl;
	}
}
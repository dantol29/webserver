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
	checkSocketOptions();
	bindToPort(_port);
	listen();
}

void Server::startPollEventLoop()
{
	addServerSocketPollFdToVectors();
	while (1)
	{
		std::cout << "++++++++++++++ Waiting for new connection or Polling +++++++++++++++" << std::endl;
		// std::cout << "printFDsVector(_FDs); - before polling" << std::endl;
		// printFDsVector(_FDs);
		int ret = poll(_FDs.data(), _FDs.size(), -1);
		// std::cout << "printFDsVector(_FDs); - after polling" << std::endl;
		// printFDsVector(_FDs);
		// print_connectionsVector(_connections);
		std::cout << "poll() returned: " << ret << std::endl;
		if (ret > 0)
		{
			for (size_t i = 0; i < _FDs.size(); i++)
			{
				std::cout << "i: " << i << std::endl;
				if (_FDs[i].revents & POLLIN)
				{
					// std::cout << "POLLIN" << std::endl;
					if (i == 0)
					{
						std::cout << "Server socket event" << std::endl;
						// std::cout << "i == 0" << std::endl;

						acceptNewConnection();
						printFDsVector(_FDs);
						print_connectionsVector(_connections);
					}
					else
					{
						std::cout << "Client socket event" << std::endl;
						// std::cout << "i != 0" << std::endl;
						// TODO: only the index is actually needed
						// handleConnection(_connections[i]);
						handleConnection(_connections[i]);
						printFDsVector(_FDs);
						print_connectionsVector(_connections);
						// _FDs.erase(_FDs.begin() + i);
						// --i;
					}
				}
				else if (_FDs[i].revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					if (i == 0)
					{
						handleServerSocketError();
					}
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

void Server::handleConnection(Connection conn)
{
	std::cout << "\nhandleConnection" << std::endl;
	conn.printConnection();
	// std::string headers;
	// HTTPResponse response;
	if (!conn.readHeaders())
	{
		// closeClientConnection(clientFD, response);
		// think if this should be also a method of the Connection class
		closeClientConnection(conn.getPollFd().fd, conn.getResponse());
		return;
	}
	std::string body;
	// isChunked is a 'free' function but it could be a method of the Connection class
	if (conn.isChunked())
	{
		if (!conn.readChunkedBody())
		{
			closeClientConnection(conn.getPollFd().fd, conn.getResponse());
			return;
		}
	}
	else
	{
		if (!conn.readBody())
		{
			closeClientConnection(conn.getPollFd().fd, conn.getResponse());
			return;
		}
	}
	// It should be double "\r\n" to separate the headers from the body
	std::string httpRequestString = conn.getHeaders() + "\r\n\r\n" + conn.getBody();
	std::cout << "Received HTTP request: " << std::endl << httpRequestString << std::endl;
	HTTPRequest request(httpRequestString.c_str());
	std::cout << request.getStatusCode() << std::endl;
	std::cout << "Received HTTP request: " << std::endl << httpRequestString << std::endl;
	printVariablesHeadersBody(request);

	// std::string response;
	Router router;
	HTTPResponse response;
	std::string _webRoot = getWebRoot();
	// Check if this is the right way to do it
	response = conn.getResponse();
	if (!router.pathExists(request, response, _webRoot))
	{
		std::cout << "Path does not exist" << std::endl;
		StaticContentHandler staticHandler;
		response = staticHandler.handleNotFound();
	}
	else
	{
		std::cout << "Path exists" << std::endl;

		// if (router.isDynamicRequest(request))
		// {
		if (request.getMethod() == "GET" && request.getRequestTarget() == "/hello.cgi")
		{
			CGIHandler cgiInstance;
			Environment env;
			env.HTTPRequestToMetaVars(request, env);
			response = cgiInstance.handleRequest(request);
		}
		else
		{
			std::cout << "Dynamic request not supported" << std::endl;
		}
		// }
		// else
		// {
		// 	StaticContentHandler staticContentHandler;
		// 	// This if condition only for legacy reasons! TODO: remove
		// 	if (request.getMethod() == "GET" &&
		// 		(request.getRequestTarget() == "/" || request.getRequestTarget() == "/home"))
		// 	{
		// 		response = staticContentHandler.handleHomePage();
		// 	}
		// 	else
		// 	{
		// 		response = staticContentHandler.handleRequest(request);
		// 	}
		// }
	}
	std::string responseString = response.getBody();

	write(conn.getPollFd().fd, responseString.c_str(), responseString.size());
	close(conn.getPollFd().fd);
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
	std::cout << "Server socket created" << std::endl;
	std::cout << "Server socket file descriptor: " << _serverFD << std::endl;
}

void Server::setReuseAddrAndPort()
{
	int opt = 1;
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEADDR: Protocol not available, continuing without SO_REUSEADDR");
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
		perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
	std::cout << "SO_REUSEADDR and SO_REUSEPORT set" << std::endl;
}

void Server::bindToPort(int port)
{
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_serverAddr.sin_port = htons(port);
	std::memset(_serverAddr.sin_zero, '\0', sizeof _serverAddr.sin_zero);

	if (bind(_serverFD, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
		perrorAndExit("In bind");
	std::cout << "Server socket bound to port " << port << std::endl;
	std::cout << "Server socket address: " << inet_ntoa(_serverAddr.sin_addr) << std::endl;
	std::cout << "Server socket port: " << ntohs(_serverAddr.sin_port) << std::endl;
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
	_FDs.push_back(serverPollFd);
	Connection serverConnection(serverPollFd, *this);
	_connections.push_back(serverConnection);
}

void Server::acceptNewConnection()
{
	// TODO: think about naming.
	// We have 4 different names for kind of the same thing: clientAddress, newSocket, newSocketPoll, newConnection
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
		Connection newConnection(newSocketPoll, *this);
		/* start together */
		_FDs.push_back(newSocketPoll);
		_connections.push_back(newConnection);
		/* end together */
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
	// TODO: should we close it with the Destructor of the Connection class?
	close(clientFD);
}

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
		std::cout << "SO_REUSEADDR is " << (optval ? "enabled" : "disabled") << std::endl;
	}

	if (getsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen) < 0)
	{
		perror("getsockopt SO_REUSEPORT failed");
	}
	else
	{
		std::cout << "SO_REUSEPORT is " << (optval ? "enabled" : "disabled") << std::endl;
	}
}

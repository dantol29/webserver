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
						handleConnection(_connections[i]);
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

void Server::handleConnection(Connection conn)
{
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

	HTTPRequest request(httpRequestString.c_str());
	std::cout << request.getStatusCode() << std::endl;
	std::cout << "Received HTTP request: " << std::endl << httpRequestString << std::endl;

	// test to execute the python script (see: https://www.tutorialspoint.com/python/python_cgi_programming.htm)
	const char *argv[] = {"./cgi-bin/hello_py.cgi", NULL};
	// const char* argv[] = { "./cgi-bin/thirty_py.cgi", NULL };
	// const char* argv[] = { "./cgi-bin/hello.cgi", NULL };

	// std::string response;
	Router router;
	HTTPResponse response;
	// Check if this is the right way to do it
	response = conn.getResponse();
	if (!router.pathExists(response, request.getRequestTarget()))
	{
		StaticContentHandler staticHandler;
		response = staticHandler.handleNotFound();
	}
	else
	{

		if (router.isDynamicRequest(request))
		{
			if (request.getMethod() == "GET" && request.getRequestTarget() == "/hello")
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
				env.setVar("request.getQueryString()", "request.getBody()");
				// response = cgiHandler.handleCGIRequest(argv, request);
				// cgiHandler.executeCGI(argv, env);
				handleCGIRequest(argv, env);
			}
		}
		else
		{
			StaticContentHandler staticContentHandler;
			// This if condition only for legacy reasons! TODO: remove
			if (request.getMethod() == "GET" &&
				(request.getRequestTarget() == "/" || request.getRequestTarget() == "/home"))
			{
				response = staticContentHandler.handleHomePage();
			}
			else
			{
				response = staticContentHandler.handleRequest(request);
			}
		}
	}
	std::string responseString = response.toString();

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
	perror("poll server socket error");
}

void Server::handleClientSocketError(int clientFD, size_t &i)
{
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

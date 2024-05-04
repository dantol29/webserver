#include "Server.hpp"
#include "Parser.hpp"
#include "Connection.hpp"

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
	int pollCounter = 0;
	while (1)
	{

		std::cout << YELLOW << "\n\n++++++++++++++ Printing _FDs right before polling +++++++++++++++" << RESET
				  << std::endl;
		printFDsVector(_FDs);
		print_connectionsVector(_connections);
		std::cout << CYAN << "++++++++++++++ #" << pollCounter
				  << " Waiting for new connection or Polling +++++++++++++++" << RESET << std::endl;
		int ret = poll(_FDs.data(), _FDs.size(), -1);
		++pollCounter;
		std::cout << "\nPoll event detected" << std::endl;
		std::cout << YELLOW << "++++++++++++++ Printing _FDs right after polling +++++++++++++++" << RESET << std::endl;
		printFDsVector(_FDs);
		print_connectionsVector(_connections);
		std::cout << YELLOW << "++++++++++++++ ++++++++++++++++++++++ +++++++++++++++" << RESET << std::endl;
		std::cout << "ret: " << ret << std::endl;
		if (ret > 0)
		{
			size_t originalSize = _FDs.size();
			std::cout << "originalSize: " << originalSize << std::endl;
			//  i < originalSize && i < _FDs.size() means that we will iterate over the _FDs vector for max the origianl
			//  size of _FDs before starting the loop. If new fds are added during the loop, they will not be considered
			//  in the loop. But if _FDs become smaller during the loop then we want to use the actual size of _FDs and
			//  not the orginal one (which is bigger).
			for (size_t i = 0; i < originalSize && i < _FDs.size(); i++)
			{
				std::cout << "for loop" << std::endl;
				std::cout << "i = " << i << " // _FD.size() " << _FDs.size() << std::endl;
				std::cout << "#" << i << " revents: " << _FDs.data()->revents << std::endl;
				if (_FDs[i].revents & (POLLIN | POLLOUT))
				{
					if (i == 0)
					{
						std::cout << "Server socket event" << std::endl;
						acceptNewConnection();
					}
					else
					{
						std::cout << "Client socket event" << std::endl;
						handleConnection(_connections[i],
										 i,
										 _connections[i].getParser(),
										 _connections[i].getRequest(),
										 _connections[i].getResponse());
						// TODO: clean this dirt!
						// add comments
						if (_connections[i].getHasFinishedReading() && _connections[i].getHasDataToSend())
							_FDs[i].events = POLLOUT;
						printFDsVector(_FDs);
						print_connectionsVector(_connections);
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

void createFile(HTTPRequest &request)
{
	std::vector<File> files = request.getFiles();
	std::string filename;
	std::map<std::string, std::string>::iterator it = files.back().headers.find("filename");

	if (it != files.back().headers.end())
		filename = it->second;
	else
	{
		std::cout << "Error: file does not have a name" << std::endl;
		return;
	}

	std::ofstream outfile(filename.c_str());
	if (outfile.is_open())
	{
		outfile << files.back().fileContent;
		outfile.close();
		std::cout << "File created successfully" << std::endl;
	}
	else
		std::cout << "Error opening file" << std::endl;
}

void Server::readFromClient(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	(void)i;
	std::cout << "\033[1;36m"
			  << "Entering readFromClient"
			  << "\033[0m" << std::endl;
	// TODO: change to _areHeadersCopmplete
	if (!parser.getHeadersComplete())
	{
		std::cout << "\033[1;33m"
				  << "Reading headers"
				  << "\033[0m" << std::endl;
		if (!conn.readHeaders(parser))
		{
			std::cout << "Error reading headers" << std::endl;
			conn.setHasFinishedReading(true);
			conn.setHasDataToSend(false);
			conn.setCanBeClosed(true);
			return;
		}
		conn.setHasReadSocket(true);
		if (!parser.preParseHeaders(response))
		{
			conn.setCanBeClosed(true);
			conn.setHasFinishedReading(true);
			conn.setHasDataToSend(true);
			std::cout << "Error pre-parsing headers" << std::endl;
			return;
		}
	}
	if (!parser.getHeadersComplete())
	{
		std::cout << "Headers incomplete yet, exiting readFromClient." << std::endl;
		return;
	}
	if (parser.getHeadersComplete() && !parser.getHeadersAreParsed())
		parser.parseRequestLineAndHeaders(parser.getHeadersBuffer().c_str(), request, response);

	std::cout << parser.getHeadersComplete() << " ," << request.getMethod() << std::endl;
	if (parser.getHeadersComplete() && request.getMethod() == "GET")
	{
		std::cout << "-------------------------Enter what we need" << std::endl;
		conn.setHasFinishedReading(true);
	}

	if (response.getStatusCode() != 0)
		std::cout << "Error: " << response.getStatusCode() << std::endl;
	if (request.getMethod() == "GET")
		std::cout << "GET request, no body to read" << std::endl;
	else
	{
		if (parser.getIsChunked() && !conn.getHasReadSocket())
		{
			std::cout << "Chunked body" << std::endl;
			if (!conn.readChunkedBody(parser))
			{
				// Case of error while reading chunked body
				conn.setCanBeClosed(true);
				conn.setHasFinishedReading(true);
				// It could be that we had data that could be sent even if we have an error cause previous data was read
				return;
			}
			conn.setHasReadSocket(true);
		}
		else
		{
			std::cout << "\033[1;33m"
					  << "Reading body"
					  << "\033[0m" << std::endl;
			// TODO: add comments
			if (!parser.getBodyComplete() && parser.getBuffer().size() == request.getContentLength())
			{
				// TODO: in the new design we will return here and go to the function where the response is
				parser.setBodyComplete(true);
				conn.setHasFinishedReading(true);
				conn.setHasDataToSend(true);
			}
			else if (!conn.getHasReadSocket() && !conn.readBody(parser, request, response))
			{
				std::cout << "Error reading body" << std::endl;
				conn.setCanBeClosed(true);
				conn.setHasFinishedReading(true);
				// Probably hasDataToSend false, because we have an error on reading the body
				// conn.setHasDataToSend(false);
				return;
			}
		}
		if (!parser.getBodyComplete())
		{
			std::cout << "Body still incomplete, exiting readFromClient." << std::endl;
			return;
		}
		if (!request.getUploadBoundary().empty())
			parser.parseFileBody(parser.getBuffer(), request, response);
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
	std::cout << "\033[1;36m"
			  << "Entering buildResponse"
			  << "\033[0m" << std::endl;
	std::cout << "\033[1;91mRequest status code: " << response.getStatusCode() << "\033[0m" << std::endl;
	if (response.getStatusCode() != 0)
	{
		response.setErrorResponse(response.getStatusCode());
		conn.setHasDataToSend(true);
		return;
	}
	if (!request.getUploadBoundary().empty())
	{
		createFile(request);
	}
	std::string responseString;
	// std::cout << request.getRequestTarget() << std::endl;
	// TODO: The Router should be a member of the Server class or of the Connection class
	Router router;
	response = router.routeRequest(request);
	responseString = response.objToString();
	conn.setHasDataToSend(true);
}

void Server::writeToClient(Connection &conn, size_t &i, HTTPResponse &response)
{
	(void)i;
	std::cout << "\033[1;36m"
			  << "Entering writeToClient"
			  << "\033[0m" << std::endl;
	std::cout << "Response: " << response.objToString() << std::endl;
	// send(conn.getPollFd().fd, response.objToString().c_str(), response.objToString().size(), 0);
	if (send(conn.getPollFd().fd, response.objToString().c_str(), response.objToString().size(), 0) < 0)
	{
		perror("send failed");
		conn.setCanBeClosed(true);
		conn.setHasFinishedSending(true);
		return;
	}
	std::cout << "Response sent" << std::endl;
	// conn.setHasDataToSend(); will not be always false in case of chunked response or keep-alive connection
	conn.setHasDataToSend(false);
	conn.setHasFinishedSending(true);
	// setCanBeClosed(true) would not be the case only if we have a keep-alive connection or a chunked response
	conn.setCanBeClosed(true);
}

void Server::closeClientConnection(Connection &conn, size_t &i)
{

	std::cout << "\033[1;36m"
			  << "Entering closeClientConnection"
			  << "\033[0m" << std::endl;
	// if (response.getStatusCode() != 0)
	// if (conn.getResponse().getStatusCode() != 0 && conn.getResponse().getStatusCode() != 499)
	// {
	// 	std::string responseString = conn.getResponse().objToString();
	// 	send(conn.getPollFd().fd, responseString.c_str(), responseString.size(), 0);
	// }
	// TODO: should we close it with the Destructor of the Connection class?
	std::cout << RED << "Connection fd: " << conn.getPollFd().fd << RESET << std::endl;
	std::cout << "i = " << i << std::endl;
	std::cout << "size of _FDs: " << _FDs.size() << std::endl;
	std::cout << "_FDs" << std::endl;
	printFDsVector(_FDs);
	std::cout << "size of _connections: " << _connections.size() << std::endl;
	std::cout << "_connections" << std::endl;
	std::cout << "Inside closeClientConnection START - before closing end erasing" << std::endl;
	print_connectionsVector(_connections);
	std::cout << "Inside closeClientConnection STOP" << std::endl;
	// close(conn.getPollFd().fd);
	if (close(conn.getPollFd().fd) < 0)
		perror("close failed");
	std::cout << " Connection closed: FD: " << conn.getPollFd().fd << std::endl;
	std::cout << "after close" << std::endl;
	_FDs.erase(_FDs.begin() + i);
	std::cout << "after erase _FDs" << std::endl;
	_connections.erase(_connections.begin() + i);
	std::cout << "after erase _connections" << std::endl;
	std::cout << "Inside closeClientConnection START - before exiting" << std::endl;
	print_connectionsVector(_connections);
	std::cout << "Inside closeClientConnection STOP - before exiting" << std::endl;
	--i;
}

void Server::handleConnection(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	std::cout << "\033[1;36m"
			  << "Entering handleConnection"
			  << "\033[0m" << std::endl;
	conn.printConnection();

	conn.setHasReadSocket(false);
	std::cout << "after conn.setHasReadSocket(false);" << std::endl;

	if (!conn.getHasFinishedReading())
		readFromClient(conn, i, parser, request, response);
	// TODO: add comments to explain
	// std::cout << "Print request" << std::endl;
	// std::cout << request << std::endl;
	std::cout << "after readFromClient" << std::endl;
	if (conn.getHasReadSocket() && !conn.getHasFinishedReading())
		return;
	if (!conn.getCanBeClosed() && !conn.getHasDataToSend())
		buildResponse(conn, i, request, response);
	std::cout << "after buildResponse" << std::endl;
	if (conn.getHasDataToSend())
		writeToClient(conn, i, response);
	std::cout << "after writeToClient" << std::endl;
	if (conn.getCanBeClosed())
		closeClientConnection(conn, i);
	std::cout << "after closeClientConnection" << std::endl;
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
	_FDs.push_back(serverPollFd);
	Connection serverConnection(serverPollFd, *this);
	_connections.push_back(serverConnection);
}

void Server::acceptNewConnection()
{
	// TODO: think about naming.
	// We have 4 different names for kind of the same thing: clientAddress, newSocket, newSocketPoll,
	// newConnection
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
		// std::cout << "New connection file descriptor: " << newSocket << std::endl;
		// std::cout << "New connection _hasReadSocket: " << newConnection.getHasReadSocket() << std::endl;
		// std::cout << "New connection _hasFinishedReading: " << newConnection.getHasFinishedReading() << std::endl;
		// std::cout << "New connection _hasDataToSend: " << newConnection.getHasDataToSend() << std::endl;
		// std::cout << "New connection _hasFinishedSending: " << newConnection.getHasFinishedSending() << std::endl;
		// std::cout << "New connection _canBeClosed: " << newConnection.getCanBeClosed() << std::endl;
		newConnection.printConnection();
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

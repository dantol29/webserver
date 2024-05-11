#include "Server.hpp"
#include "Parser.hpp"
#include "Connection.hpp"
#include "ServerBlock.hpp" // for the Listen struct (to be implemented)
#include "Debug.hpp"

Server::Server()
{
	loadDefaultConfig();
	Debug::log("Server created with defaut constructor", Debug::OCF);
}

Server::Server(const Config &config)
{
	_config = config;
	// while we don't have a config file
	loadDefaultConfig();
	Debug::log("Server created with config constructor", Debug::OCF);
}

Server::~Server()
{
	Debug::log("Server destroyed", Debug::OCF);
}

void Server::startListening()
{
	// createServerSocket();
	createServerSockets(_serverFDs, _config.getServerBlocks());
	// setReuseAddrAndPort();
	setReuseAddrAndPort(_serverFDs);
	// checkSocketOptions();
	checkSocketOptions(_serverFDs);
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
					// if (i == 0)
					if (_connections[i].getType() == SERVER)
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
			conn.setCanBeClosed(true);
			conn.setHasFinishedReading(true);
			conn.setHasDataToSend(true);
			Debug::log("Error pre-parsing headers", Debug::OCF);
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
			else if (!conn.getHasReadSocket() && !conn.readBody(parser, request, response))
			{
				Debug::log("Error reading body", Debug::OCF);
				conn.setCanBeClosed(true);
				conn.setHasFinishedReading(true);
				// Probably hasDataToSend false, because we have an error on reading the body
				// conn.setHasDataToSend(false);
				return;
			}
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
	Debug::log("\033[1;36mEntering buildResponse\033[0m", Debug::OCF);
	if (response.getStatusCode() != 0)
	{
		response.setErrorResponse(response.getStatusCode());
		conn.setHasDataToSend(true);
		return;
	}
	// std::cout << request.getRequestTarget() << std::endl;
	// TODO: The Router should be a member of the Server class or of the Connection class
	Router router;
	router.setFDsRef(&_FDs);
	router.setPollFd(&conn.getPollFd());
	router.routeRequest(request, response);
	conn.setHasDataToSend(true);
}

void Server::writeToClient(Connection &conn, size_t &i, HTTPResponse &response)
{
	Debug::log("\033[1;36mEntering writeToClient\033[0m", Debug::OCF);
	(void)i;
	send(conn.getPollFd().fd, response.objToString().c_str(), response.objToString().size(), 0);
	// conn.setHasDataToSend(); will not be always false in case of chunked response or keep-alive connection
	conn.setHasDataToSend(false);
	conn.setHasFinishedSending(true);
	// setCanBeClosed(true) would not be the case only if we have a keep-alive connection or a chunked response
	conn.setCanBeClosed(true);
}

void Server::closeClientConnection(Connection &conn, size_t &i)
{
	Debug::log("\033[1;36mEntering closeClientConnection\033[0m", Debug::OCF);

	// TODO: should we close it with the Destructor of the Connection class?
	close(conn.getPollFd().fd);
	_FDs.erase(_FDs.begin() + i);
	_connections.erase(_connections.begin() + i);
	--i;
}

void Server::handleConnection(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	Debug::log("\033[1;36mEntering handleConnection\033[0m", Debug::OCF);
	// conn.printConnection();

	// Why is it TRUE when I refresh a page?????
	conn.setHasReadSocket(false);
	std::cout << "Has finished reading: " << conn.getHasFinishedReading() << std::endl;
	if (!conn.getHasFinishedReading())
		readFromClient(conn, i, parser, request, response);
	// TODO: add comments to explain
	if (conn.getHasReadSocket() && !conn.getHasFinishedReading())
	{
		Debug::log("\033[1;36mreturn from handleConnection\033[0m", Debug::OCF);
		return;
	}
	if (!conn.getCanBeClosed() && !conn.getHasDataToSend())
		buildResponse(conn, i, request, response);
	// std::cout << "Has data to send: " << conn.getHasDataToSend() << std::endl;
	// std::cout << response << std::endl;
	if (conn.getHasDataToSend())
		writeToClient(conn, i, response);
	// std::cout << "Has finished sending: " << conn.getHasFinishedSending() << std::endl;
	// std::cout << "Can be closed: " << conn.getCanBeClosed() << std::endl;
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

std::string normalizeIPAddress(const std::string &ip, bool isIpV6)
{
	if (isIpV6)
	{
		size_t pos = ip.find("::ffff:");
		if (pos != std::string::npos)
			return ip.substr(pos + 7); // Remove the ::ffff:
		pos = ip.find("::");
		if (pos != std::string::npos && ip.length() - pos == 7)
			return ip.substr(pos + 2); // Remove the ::
	}
	return ip;
}

void Server::createServerSockets(std::vector<int> _serverFDs, std::vector<ServerBlock> serverBlocks)
{
	std::vector<Listen> allListens;
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
	{
		std::vector<Listen> listens = it->getDirectives().getListen();
		allListens.insert(allListens.end(), listens.begin(), listens.end());
	}

	std::vector<Listen> uniqueListens;
	for (std::vector<Listen>::iterator it = allListens.begin(); it != allListens.end(); ++it)
	{
		std::string normalizedIp = normalizeIPAddress(it->_ip, it->_isIpv6);
		bool isUnique = true;
		for (std::vector<Listen>::iterator it2 = uniqueListens.begin(); it2 != uniqueListens.end(); ++it2)
		{
			std::string normalizedIp2 = normalizeIPAddress(it2->_ip, it2->_isIpv6);
			if (it->_port == it2->_port && normalizedIp == normalizedIp2)
			{
				isUnique = false;
				break;
			}
		}
		if (isUnique)
			uniqueListens.push_back(*it);
	}
	for (std::vector<Listen>::iterator it = uniqueListens.begin(); it != uniqueListens.end(); ++it)
	{
		int serverFD;
		if ((serverFD = socket(AF_INET6, SOCK_STREAM, 0)) == 0)
		{
			perror("Failed to create server socket");
			continue; // just to remember that we aren not exiting
		}
		int no = 0;
		if (setsockopt(serverFD, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) < 0)
		{
			perror("setsockopt IPV6_V6ONLY: Protocol not available, continuing without IPV6_V6ONLY");
			continue; // just to remember that we aren not exiting
		}
		_serverFDs.push_back(serverFD);
	}
}

void Server::setReuseAddrAndPort(std::vector<int> _serverFDs)
{
	int opt = 1;
	for (std::vector<int>::iterator it = _serverFDs.begin(); it != _serverFDs.end(); ++it)
	{
		if (setsockopt(*it, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
			perror("setsockopt SO_REUSEADDR: Protocol not available, continuing without SO_REUSEADDR");
		if (setsockopt(*it, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
			perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
	}
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

	// struct sockaddr_in clientAddress;
	// We choose sockaddr_storage to be able to handle both IPv4 and IPv6
	struct sockaddr_storage clientAddress;
	socklen_t ClientAddrLen = sizeof(clientAddress);
	Debug::log("New connection detected", Debug::SERVER);
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
		char clientIP[INET6_ADDRSTRLEN];
		if (clientAddress.ss_family == AF_INET)
		{
			struct sockaddr_in *s = (struct sockaddr_in *)&clientAddress;
			// TODO: inet_ntop is forbidden in the subject.
			inet_ntop(AF_INET, &s->sin_addr, clientIP, sizeof clientIP);
			std::cout << "New connection from (IPv4): " << clientIP << std::endl;
		}
		else if (clientAddress.ss_family == AF_INET6)
		{
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)&clientAddress;
			inet_ntop(AF_INET6, &s->sin6_addr, clientIP, sizeof clientIP);
			std::cout << "New connection from (IPv6): " << clientIP << std::endl;
		}
		else
		{
			std::cerr << "Unknown address family" << std::endl;
		}
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

void Server::checkSocketOptions(std::vector<int> _serverFDs)
{
	int optval;
	socklen_t optlen = sizeof(optval);

	for (std::vector<int>::iterator it = _serverFDs.begin(); it != _serverFDs.end(); ++it)
	{
		if (getsockopt(*it, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen) < 0)
		{
			perror("getsockopt SO_REUSEADDR failed for server socket");
		}
		else
		{
			// std::cout << "SO_REUSEADDR is " << (optval ? "enabled" : "disabled") << std::endl;
		}

		if (getsockopt(*it, SOL_SOCKET, SO_REUSEPORT, &optval, &optlen) < 0)
		{
			perror("getsockopt SO_REUSEPORT failed for server socket");
		}
		else
		{
			// std::cout << "SO_REUSEPORT is " << (optval ? "enabled" : "disabled") << std::endl;
		}
	}
}

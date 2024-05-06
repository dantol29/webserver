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
	while (1)
	{
		std::cout << "++++++++++++++ Waiting for new connection or Polling +++++++++++++++" << std::endl;
		int ret = poll(_FDs.data(), _FDs.size(), -1);
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
					std::cout << "i: " << i << std::endl;
					std::cout << "Enters revents" << std::endl;
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

						// it is NOT CORRECT because we do i-- in closeConnection
						// if (_connections[i].getHasFinishedReading() \
					// && _connections[i].getHasDataToSend())
						//_FDs[i].events = POLLOUT;
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
	std::vector<File>::iterator it;

	// check each file
	for (it = files.begin(); it != files.end(); ++it)
	{
		if (it->headers.find("filename") == it->headers.end())
		{
			std::cout << "422 Unprocessable Entity (Error: file does not have a name)" << std::endl;
			return ;
		}

	}

	// create files
	for (it = files.begin(); it != files.end(); ++it)
	{
		std::ofstream outfile((it->headers.find("filename"))->second.c_str());
		if (outfile.is_open())
		{
			outfile << it->fileContent;
			outfile.close();
			std::cout << "File created successfully" << std::endl;
		} 
		else
			std::cout << "422 Unprocessable Entity (Error creating a file)" << std::endl;
	}
}

void Server::readFromClient(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	(void)i;
	std::cout << "\033[1;36m" << "Entering readFromClient" << "\033[0m" << std::endl;
	// TODO: change to _areHeadersCopmplete
	if (!parser.getHeadersComplete())
	{
		std::cout << "\033[1;33m" << "Reading headers" << "\033[0m" << std::endl;
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
			std::cout << "\033[1;33m" << "Reading body" << "\033[0m" << std::endl;
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
			conn.setHasFinishedReading(false);
			conn.setHasReadSocket(true);
			return;
		}
		//std::cout << parser.getBuffer() << std::endl;
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
	std::cout << "\033[1;36m" << "Entering buildResponse" << "\033[0m" << std::endl;
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
	std::cout << "\033[1;36m" << "Entering writeToClient" << "\033[0m" << std::endl;
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
	std::cout << "\033[1;36m" << "Entering closeClientConnection" << "\033[0m" << std::endl;
	// if (response.getStatusCode() != 0)
	// if (conn.getResponse().getStatusCode() != 0 && conn.getResponse().getStatusCode() != 499)
	// {
	// 	std::string responseString = conn.getResponse().objToString();
	// 	send(conn.getPollFd().fd, responseString.c_str(), responseString.size(), 0);
	// }
	// TODO: should we close it with the Destructor of the Connection class?
	close(conn.getPollFd().fd);
	_FDs.erase(_FDs.begin() + i);
	_connections.erase(_connections.begin() + i);
	--i;
}

void Server::handleConnection(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response)
{
	std::cout << "\033[1;36m" << "Entering handleConnection" << "\033[0m" << std::endl;
	// conn.printConnection();

	// Why is it TRUE when I refresh a page?????
	conn.setHasReadSocket(false);
	std::cout << "Has finished reading: " << conn.getHasFinishedReading() << std::endl;
	if (!conn.getHasFinishedReading())
		readFromClient(conn, i, parser, request, response);
	// TODO: add comments to explain
	if (conn.getHasReadSocket() && !conn.getHasFinishedReading()){
		std::cout << "\033[1;36m" << "return from handleConnection" << "\033[0m" << std::endl;
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
		std::cout << newConnection.getHasFinishedReading() << std::endl;
		std::cout << _connections.back().getHasFinishedReading() << std::endl;
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

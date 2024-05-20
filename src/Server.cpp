#include "Server.hpp"
#include "Parser.hpp"
#include "Connection.hpp"
#include "ServerBlock.hpp"
#include "Debug.hpp"
#include "EventManager.hpp"
#include "signal.h"

Server::Server(const Config &config, EventManager &eventManager) : _config(config), _eventManager(eventManager)
{
	_maxClients = 10;
	_clientMaxHeadersSize = CLIENT_MAX_HEADERS_SIZE;
	_serverBlocks = _config.getServerBlocks();
	_serverSockets = std::vector<ServerSocket>();
	_hasCGI = false;
	_CGICounter = 0;
	Debug::log("Server created with config constructor", Debug::OCF);
}

Server::~Server()
{
	Debug::log("Server destroyed", Debug::OCF);
}

// GETTERS AND SETTERS

void Server::setHasCGI(bool hasCGI)
{
	_hasCGI = hasCGI;
}

void Server::setCGICounter(int counter)
{
	_CGICounter = counter;
}

bool Server::getHasCGI() const
{
	return _hasCGI;
}

int Server::getCGICounter() const
{
	return _CGICounter;
}

// METHODS

void Server::startListening()
{
	createServerSockets(_serverBlocks);
	printServerSockets();
	setReuseAddrAndPort();
	checkSocketOptions();
	bindToPort();
	listen();
}

void Server::startPollEventLoop()
{
	addServerSocketsPollFdToVectors();
	int pollCounter = 0;
	int timeout = -1;
	while (1)
	{
		if (_hasCGI)
			timeout = 500;
		else
			timeout = -1;
		// printConnections("BEFORE POLL", _FDs, _connections, true);
		std::cout << CYAN << "++++++++++++++ #" << pollCounter
				  << " Waiting for new connection or Polling +++++++++++++++" << RESET << std::endl;
		int ret = poll(_FDs.data(), _FDs.size(), timeout);
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
						std::cout << "Has finished reading: " << _connections[i].getHasFinishedReading() << std::endl;
						std::cout << "Has data to send: " << _connections[i].getHasDataToSend() << std::endl;
						std::cout << "Has CGI: " << _connections[i].getHasCGI() << std::endl;
						// If a Connection has a CGI it means it has already finished reading, cuase the hasCGI is set
						// after
						// if ((_connections[i].getHasFinishedReading() && _connections[i].getHasDataToSend()) ||
						// 	_connections[i].getHasCGI())
						_FDs[i].events = POLLOUT;
						if ((_connections[i].getHasFinishedReading() && _connections[i].getHasDataToSend()))
						{
							std::cout << "Setting POLLOUT" << std::endl;
							_FDs[i].events = POLLOUT;
						}
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
		std::cout << "Before if(_hasCGI)" << std::endl;
		// We are checking if the server has CGI. And it should be true. Is our event handler working?
		std::cout << "Has CGI: " << (_hasCGI ? "true" : "false") << std::endl;
		if (_hasCGI)
		{
			std::cout << "We enter the hasCGI loop" << std::endl;
			size_t originalSize = _FDs.size();
			int status;
			pid_t pid = waitpid(-1, &status, WNOHANG);
			std::cout << "PID: " << pid << std::endl;
			if (pid > 0)
			{
				// TODo: has at this point the CGI been executed?
				for (size_t i = 0; i < originalSize && i < _FDs.size(); i++)
				{
					if (_connections[i].getHasCGI() && _connections[i].getCGIPid() == pid)
					{
						_connections[i].setCGIExitStatus(status);
						_connections[i].removeCGI(status);
						// We still need to read the buffer before we can send the response
						// We assume that the CGI has been executed and we can set the FD to POLLOUT and has data to
						// Mind theat the buffer needs to be read before we can send the response
						// send kill(_connections[i].getCGIPid(), SIGKILL);
						std::cout << "Setting POLLOUT" << std::endl;
						_FDs[i].events = POLLOUT;
						break;
					}
				}
				// We deccrement the CGI counter by 1 on the server and if it 0 we set _hasCGI to false
				removeCGI();
			}
			else if (pid == 0)
			{
				// Check if the CGI has timed out
				for (size_t i = 0; i < originalSize && i < _FDs.size(); i++)
				{
					_connections[i].setCGIExitStatus(1);
					if (_connections[i].getHasCGI() && _connections[i].getCGIStartTime() + CGI_TIMEOUT_MS > time(NULL))
					{
						// kill CGI process
						std::cout << "CGI timeout" << std::endl;
						// probably we need here also removeCGI and go back to hanldeRequest and stuff
						_FDs[i].events = POLLOUT;
						kill(_connections[i].getCGIPid(), SIGKILL);
						// give a response back that the CGI timeout
					}
				}
			}
			else
				perror("waitpid");
		}
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
				// It could be that we had data that could be sent even if we have an error cause previous data
				// was read
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
	// This couldl be stramlined with
	// if (response.getIsCGI()
	// {
	// 	buildCGIResponse(conn, response);
	// 	return ;
	// }
	// It makes more sense to have it at the end and clean up thsi huge method
	if (!response.getIsCGI())
	{

		Debug::log("Entering buildResponse", Debug::NORMAL);
		Debug::log("Request method: " + request.getMethod(), Debug::NORMAL);

		ServerBlock serverBlock;
		Directives directive;
		std::string serverName;
		std::cout << GREEN << "Number of server blocks: " << _config.getServerBlocks().size() << RESET << std::endl;
		std::cout << "Request host: " << request.getSingleHeader("host").second << std::endl;
		std::cout << "Request target: " << request.getRequestTarget() << std::endl;

		std::string requestTarget = request.getRequestTarget();
		// if there is "?" in the request target, we need to remove it
		if (std::find(requestTarget.begin(), requestTarget.end(), '?') != requestTarget.end())
			requestTarget = requestTarget.substr(0, requestTarget.find("?"));
		std::cout << "Request target: " << requestTarget << std::endl;

		for (size_t i = 0; i < _config.getServerBlocks().size(); i++)
		{
			// loop through all server names in the server block
			for (size_t j = 0; j < _config.getServerBlocks()[i].getServerName().size(); j++)
			{
				serverName = _config.getServerBlocks()[i].getServerName()[j];
				std::cout << RED << "Checking server name: " << serverName << RESET << std::endl;
				if (serverName == request.getSingleHeader("host").second)
				{
					std::cout << GREEN << "Server name found" << RESET << std::endl;
					break;
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
					std::cout << "Location: " << serverBlock.getLocations()[i]._path
							  << " == " << request.getRequestTarget() << std::endl;
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

		std::string root = serverBlock.getRoot();

		std::cout << "Root: " << root << std::endl;
		if (root[root.size() - 1] != '/')
			root = root + "/";
		std::cout << RED << "Root: " << root << RESET << std::endl;

		Router router(directive, _eventManager, conn);

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
		// This wiill not be true, if the response _isCGI is true
		std::cout << "Is Response CGI? " << response.getIsCGI() << std::endl;
		if (!response.getIsCGI())
		{
			std::cout << "Setting setHasDataToSend to true" << std::endl;
			conn.setHasDataToSend(true);
			return;
		}
	}
	else
	{
		// We want to build the response from the CGI only after we went through the poll and we have the data from the
		// CGI.
		buildCGIResponse(conn, response);
	}
	std::cout << "Exiting buildResponse" << std::endl;
}
void Server::buildCGIResponse(Connection &conn, HTTPResponse &response)
{
	std::cout << RED << "Entering buildCGIResponse" << RESET << std::endl;
	std::string cgiOutput;
	int *pipeFD;
	pipeFD = response.getCGIpipeFD();
	char readBuffer[256];
	// TODO: this is blokcing - we need to make it non-blocking
	// I.e. read 1 buffer and then go back to poll
	std::cout << "Reading from pipe" << std::endl;
	ssize_t bytesRead;
	do
	{
		std::cout << "Into the do while loop" << std::endl;
		bytesRead = read(pipeFD[0], readBuffer, sizeof(readBuffer) - 1);
		std::cout << "Bytes read: " << bytesRead << std::endl;
		if (bytesRead > 0)
		{
			std::cout << "Bytes read: " << bytesRead << std::endl;
			readBuffer[bytesRead] = '\0';
			cgiOutput += readBuffer;
		}
		else if (bytesRead == 0)
		{
			std::cout << "End of data stream reached." << std::endl;
			break; // Optional: Explicitly break if you need to perform additional cleanup.
		}
		else
		{
			std::cerr << "Error reading data: " << strerror(errno) << std::endl;
			break; // Break or handle the error as needed.
		}
	} while (bytesRead > 0);

	std::cout << "CGI output: " << cgiOutput << std::endl;
	close(pipeFD[0]);

	int status = conn.getCGIExitStatus();
	//
	// if (waitedPid == -1)
	// {
	// 	perror("waitpid");
	// 	return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
	// }

	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	{
		response.setStatusCode(500, "Internal Server Error");
		conn.setHasDataToSend(true);
		response.setIsCGI(false);
		// TODO: should we set other flags here?
		return;
	}

	if (cgiOutput.empty())
	{
		response.setStatusCode(500, "Internal Server Error");
		conn.setHasDataToSend(true);
		response.setIsCGI(false);
		return;
	}
	response.CGIStringToResponse(cgiOutput);
	response.setIsCGI(false);
	conn.setHasDataToSend(true);

	// return cgiOutput;
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
	// MInd that after the last read from the pipe of the CGI getHasReadSocket will be false but we will have a read
	// operation on the pipe, if we want to write just after going through poll we need an extra flag or something.
	std::cout << "Has read socket: " << conn.getHasReadSocket() << std::endl;
	std::cout << "Has data to send: " << conn.getHasDataToSend() << std::endl;
	if (conn.getHasDataToSend() && !conn.getHasReadSocket())
		writeToClient(conn, i, response);
	std::cout << "Can be closed: " << conn.getCanBeClosed() << std::endl;
	if (conn.getCanBeClosed())
		closeClientConnection(conn, i);
	std::cout << RED << "Exiting handleConnection" << RESET << std::endl;
}

/*** Private Methods ***/
/* startListening */

std::string normalizeIPAddress(const std::string &ip, bool isIpV6)
{
	// We use this function to normalize the IPv4 adresses mapped to IPv6
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

void Server::createServerSockets(std::vector<ServerBlock> &serverBlocks)
{
	std::cout << BLUE << "Entering createServerSockets" << RESET << std::endl;
	std::vector<Listen> allListens;
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
	{
		std::vector<Listen> listens = it->getListen();
		allListens.insert(allListens.end(), listens.begin(), listens.end());
	}

	std::vector<Listen> uniqueListens;
	for (std::vector<Listen>::iterator it = allListens.begin(); it != allListens.end(); ++it)
	{
		std::string normalizedIp = normalizeIPAddress(it->getIp(), it->getIsIpv6());
		bool isUnique = true;
		for (std::vector<Listen>::iterator it2 = uniqueListens.begin(); it2 != uniqueListens.end(); ++it2)
		{
			std::string normalizedIp2 = normalizeIPAddress(it2->getIp(), it2->getIsIpv6());
			if (it->getPort() == it2->getPort() && normalizedIp == normalizedIp2)
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

		if ((serverFD = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		{
			perror("Failed to create server socket");
			continue; // just to remember that we aren not exiting
		}
		// We set the socket option IPV6_V6ONLY to 0
		int no = 0;
		if (setsockopt(serverFD, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) < 0)
		{
			perror("setsockopt IPV6_V6ONLY: Protocol not available, continuing without IPV6_V6ONLY");
			continue; // just to remember that we aren not exiting
		}
		// We check if IPV6_V6ONLY is set to 0
		int ipv6only;
		socklen_t len = sizeof(ipv6only);
		if (getsockopt(serverFD, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, &len) < 0)
		{
			perror("getsockopt IPV6_V6ONLY: Protocol not available, continuing without IPV6_V6ONLY");
			continue; // just to remember that we aren not exiting
		}
		else
		{
			std::cout << "IPV6_V6ONLY: " << ipv6only << std::endl;
		}
		ServerSocket serverSocket(serverFD, *it);
		_serverSockets.push_back(serverSocket);
	}
	std::cout << BLUE << "Server socket(s) created" << RESET << std::endl;
	std::cout << "\n\n";
}

void Server::setReuseAddrAndPort()
{
	int opt = 1;
	for (std::vector<ServerSocket>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		if (setsockopt(it->getServerFD(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
			perror("setsockopt SO_REUSEADDR: Protocol not available, continuing without SO_REUSEADDR");
		if (setsockopt(it->getServerFD(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
			perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");

		// std::cout << "SO_REUSEADDR and SO_REUSEPORT set" << std::endl;
	}
}

void Server::bindToPort()
{
	for (std::vector<ServerSocket>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		it->prepareServerSocketAddr();

		struct sockaddr_in6 serverSocketAddr = it->getServerSocketAddr();

		// Convert IPv6 address from binary to text
		char ipv6Address[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(serverSocketAddr.sin6_addr), ipv6Address, INET6_ADDRSTRLEN);

		// Print original IP Address and Port
		std::cout << "Original IP Address: " << ipv6Address << std::endl;
		std::cout << "Original Port: " << ntohs(serverSocketAddr.sin6_port) << std::endl;

		// Overwrite the IP address and port with hardcoded values for test
		// serverSocketAddr.sin6_addr = in6addr_any; // Bind to all interfaces for IPv6
		// serverSocketAddr.sin6_port = htons(8080); // Hardcoded port 8080
		// We need to cast it to sockaddr to be able to use it in bind()
		const sockaddr *serverSocketAddrPtr = reinterpret_cast<const sockaddr *>(&serverSocketAddr);

		// Print the sockaddr and address family
		// We reuse the ipv6Address buffer to store the new IP address
		inet_ntop(AF_INET6, &(serverSocketAddr.sin6_addr), ipv6Address, INET6_ADDRSTRLEN);
		std::cout << "First print of serverSocketAddr" << std::endl;
		std::cout << "IP Address: " << ipv6Address << std::endl;
		std::cout << "Port: " << ntohs(serverSocketAddr.sin6_port) << std::endl;
		std::cout << "Address Family: " << serverSocketAddr.sin6_family << std::endl;

		int bindResult = bind(it->getServerFD(), serverSocketAddrPtr, sizeof(serverSocketAddr));
		std::cout << "Bind result: " << bindResult << std::endl;
		if (bindResult < 0)
		{
			std::cout << YELLOW << "Server socket failed to bind to IP " << ipv6Address << " on port "
					  << ntohs(serverSocketAddr.sin6_port) << RESET << std::endl;
			perror("In bind");
			continue; // just to remember that we aren not exiting
		}
		else
		{
			// Convert IPv6 address from binary to text for display
			char ipv6AddressBound[INET6_ADDRSTRLEN];
			if (inet_ntop(AF_INET6, &(serverSocketAddr.sin6_addr), ipv6AddressBound, INET6_ADDRSTRLEN))
			{
				std::cout << YELLOW << "Server socket bound to IP " << ipv6AddressBound << " on port "
						  << ntohs(serverSocketAddr.sin6_port) << RESET << std::endl;
			}
			else
			{
				std::cerr << "Error converting bound IPv6 address to text." << std::endl;
			}
			std::cout << "***" << std::endl;
		}
		// if (bind(it->getServerFD(), serverSocketAddrPtr, sizeof(serverSocketAddr)) < 0)
	}
}

void Server::listen()
{
	for (std::vector<ServerSocket>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		// listen return 0 on success and -1 on error (Linux and MacOS) - Errno is set
		if (::listen(it->getServerFD(), _maxClients) == -1)
		{
			perror("In listen");
			continue; // just to remember that we aren not exiting
		}

		struct sockaddr_in6 addr = it->getServerSocketAddr(); // Assume this function exists and retrieves the address
		char ipv6Address[INET6_ADDRSTRLEN];

		// Convert IPv6 address from binary to text
		if (inet_ntop(AF_INET6, &addr.sin6_addr, ipv6Address, INET6_ADDRSTRLEN))
		{
			std::cout << "Server socket listening on IP " << ipv6Address << " and port " << ntohs(addr.sin6_port)
					  << std::endl;
		}
		else
		{
			std::cerr << "Error converting IPv6 address to text." << std::endl;
		}
	}
}

/* startPollEventsLoop */

void Server::addServerSocketsPollFdToVectors()
{
	for (std::vector<ServerSocket>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		struct pollfd serverPollFd;
		memset(&serverPollFd, 0, sizeof(serverPollFd));
		serverPollFd.fd = it->getServerFD();
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
		serverConnection.setServerIp(it->getListen().getIp());
		serverConnection.setServerPort(it->getListen().getPort());
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
void Server::checkSocketOptions()
{
	int optval;
	socklen_t optlen = sizeof(optval);

	for (std::vector<ServerSocket>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		if (getsockopt(it->getServerFD(), SOL_SOCKET, SO_REUSEADDR, &optval, &optlen) < 0)
		{
			perror("getsockopt SO_REUSEADDR failed for server socket");
		}
		else
		{
			// std::cout << "SO_REUSEADDR is " << (optval ? "enabled" : "disabled") << std::endl;
		}
	}
}

void Server::printServerSockets() const
{
	std::cout << "Server sockets:" << std::endl;
	for (std::vector<ServerSocket>::const_iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
	{
		std::cout << *it << std::endl;
	}
}

/* for CGI */
// void Server::addCGI(int eventID)
void Server::addCGI(const EventData &eventData)
{
	(void)eventData;
	setHasCGI(true);
	setCGICounter(getCGICounter() + 1);
	std::cout << "CGI added: _hasCGI set to " << _hasCGI << ", _CGICounter is now " << _CGICounter << std::endl;
}

void Server::removeCGI()
{
	setCGICounter(getCGICounter() - 1);
	if (getCGICounter() == 0)
		setHasCGI(false);
}
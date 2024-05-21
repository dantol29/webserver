#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <poll.h>
#include <fstream>
#include <sstream>
#include "MetaVariables.hpp"
#include "HTTPRequest.hpp"
#include "Connection.hpp"
#include "server_utils.hpp"
#include "webserv.hpp"
#include "Parser.hpp"
#include "Config.hpp"
#include "ServerSocket.hpp"
#include "EventManager.hpp"

#define VERBOSE 1

class Connection; // Forward declaration for circular dependencyA

class Server
{
  public:
	Server(const Config &config, EventManager &eventManager);
	~Server();

	void startListening();
	void startPollEventLoop();

	void printServerSockets() const;
	/* for CGI */
	void setHasCGI(bool hasCGI);
	void setCGICounter(int counter);
	bool getHasCGI() const;
	int getCGICounter() const;
	// clang-format off
	std::vector<std::pair<int, int> > getPipeFDs() const;
	// clang-format on
	const EventManager &getEventManager() const;

	void addCGI(const EventData &eventData);
	void addPipeFDs(int pipe0, int pipe1);
	void removeCGI();

  private:
	/* Private Attributes */
	Config _config;
	int _maxClients; // i.e. max number of pending connections
	size_t _clientMaxHeadersSize;
	std::vector<ServerBlock> _serverBlocks;
	std::vector<ServerSocket> _serverSockets;
	std::vector<struct pollfd> _FDs;
	std::vector<Connection> _connections;
	// clang-format off
	std::vector<std::pair<int, int> > _pipeFDs;
	// clang-format on
	EventManager &_eventManager;

	bool _hasCGI;

	int _CGICounter;

	/*** Private Methods ***/
	Server();
	/* for startListening */
	void checkSocketOptions();
	void createServerSockets(std::vector<ServerBlock> &serverBlocks);
	void setReuseAddrAndPort();
	void bindToPort();
	void listen();
	// void addServerSocketPollFdToVectors();
	void addServerSocketsPollFdToVectors();
	void acceptNewConnection(Connection &conn);
	void handleConnection(Connection &conn, size_t &i);
	void handleServerSocketError();
	void handleClientSocketError(int clientFD, size_t &i);
	void handleSocketTimeoutIfAny();
	void handlePollError();
	void AlertAdminAndTryToRecover();
	void waitCGI();

	/* for handleConnection */
	void readFromClient(Connection &conn, size_t &i, Parser &parser, HTTPRequest &request, HTTPResponse &response);
	void handlePostRequest(Connection &conn, Parser &parser, HTTPRequest &request, HTTPResponse &response);
	void buildResponse(Connection &conn, size_t &i, HTTPRequest &request, HTTPResponse &response);
	void buildCGIResponse(Connection &conn, HTTPResponse &response);
	void writeToClient(Connection &conn, size_t &i, HTTPResponse &response);
	void closeClientConnection(Connection &conn, size_t &i);

	/* for buildResponse */
	void formRequestTarget(HTTPRequest &request);
	void findLocationBlock(HTTPRequest &request, ServerBlock &serverBlock, Directives &directive);
	void handleServerBlockError(Connection &conn, HTTPResponse &response);
	std::string findServerName(HTTPRequest &request, ServerBlock &serverBlock);
	/* Not avaiable constructors */
	// Copy constructor
	Server(const Server &other);
	// Assignment operator
	Server &operator=(const Server &other);
};

#endif
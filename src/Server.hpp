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
#include "server_utils.hpp"
#include "Connection.hpp"
#include "webserv.hpp"

class Connection; // Forward declaration for circular dependency

class Server
{
  public:
	Server();
	Server(const std::string configFilePath);
	~Server();

	void startListening();
	void startPollEventLoop();

	int getPort() const;
	void setPort(int port);

	size_t getClientMaxHeadersSize() const;
	std::string getWebRoot() const;
	void setWebRoot(const std::string &webRoot);
	std::string getConfigFilePath() const;
	void checkSocketOptions();

  private:
	/* Private Attributes */
	int _port;
	int _serverFD;
	size_t _clientMaxHeadersSize;
	int _clientMaxBodySize;
	int _maxClients; // i.e. max number of pending connections
	std::string _configFilePath;
	std::string _webRoot;
	struct sockaddr_in _serverAddr;
	std::vector<struct pollfd> _FDs;
	std::vector<Connection> _connections;

	/*** Private Methods ***/
	/* for Constructors */
	void loadConfig();
	void loadDefaultConfig();
	/* for startListening */
	void createServerSocket();
	void setReuseAddrAndPort();
	void bindToPort(int port);
	void listen();
	/* for startPollEventLoop */
	void addServerSocketPollFdToVectors();
	void acceptNewConnection();
	void handleConnection(Connection conn, size_t &i);
	void handleServerSocketError();
	void handleClientSocketError(int clientFD, size_t &i);
	void handleSocketTimeoutIfAny();
	void handlePollFailure();
	void AlertAdminAndTryToRecover();

	/* for handleConnection */
	// void closeClientConnection(int clientFD, HTTPResponse &response);
	void closeClientConnection(Connection &conn, size_t &i);

	/* Not avaiable constructors */

	// Copy constructor
	Server(const Server &other);
	// Assignment operator
	Server &operator=(const Server &other);
};

#endif
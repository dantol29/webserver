#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>	// for std::string and memset
#include <poll.h>	// For struct pollfd
#include <unistd.h> // For close
#include "Server.hpp"
#include "webserv.hpp"
#include "server_utils.hpp"
#include "HTTPResponse.hpp" // Assuming existence of HTTPResponse class
#include "Parser.hpp"		// Assuming existence of Parser class

#define USE_CUSTOM_COPY_CONSTRUCTOR 1 // NOT THE DEFAULT COPY CONSTRUCTOR
#define USE_COPY_CONSTRUCTOR_WITH_BOOLS 1

enum ConnectionType
{
	UNDEFINED,
	CLIENT,
	SERVER,
};

class Server; // Forward declaration for circular dependency

// Add enumn connection is server or client

class Connection
{

  private:
	Parser _parser;
	HTTPRequest _request;
	HTTPResponse _response;

	// TODO: add if the connection is a client or server
	ConnectionType _connectionType;

	struct pollfd _pollFd;
	bool _hasReadSocket;
	bool _hasFinishedReading;
	bool _hasDataToSend;
	bool _hasFinishedSending;
	bool _canBeClosed;

	// Additional client state can be managed here

  public:
	Connection(struct pollfd &pollFd, Server &server);
#if USE_CUSTOM_COPY_CONSTRUCTOR
	Connection(const Connection &other); // Copy constructor
#endif
	Connection &operator=(const Connection &other); // Copy assignment operator
	~Connection();

	bool readHeaders(Parser &parser);
	bool readChunkedBody(Parser &parser);
	bool readChunkSize(std::string &line);
	bool readChunk(size_t chunkSize, std::string &chunkedData, HTTPResponse &response);
	bool readBody(Parser &parser, HTTPRequest &req, HTTPResponse &res);

	/* Getters */
	// TODO: should we return a const reference also in the case of the parser, request and response?
	Parser &getParser();
	HTTPRequest &getRequest();
	HTTPResponse &getResponse();
	struct pollfd getPollFd() const;
	bool getHasReadSocket() const;
	bool getBodyComplete() const;
	std::string getChunkData() const;
	bool getHasFinishedReading() const;
	bool getHasDataToSend() const;
	bool getHasFinishedSending() const;
	bool getCanBeClosed() const;
	ConnectionType getConnectionType() const;
	/* Setters */
	void setHasReadSocket(bool value);
	void setHeadersComplete(bool headersComplete);
	void setBodyComplete(bool bodyComplete);
	void setHeaders(const std::string &headers);
	void setChunkData(const std::string &chunkData);
	void setHasFinishedReading(bool value);
	void setCanBeClosed(bool value);
	void setHasDataToSend(bool value);
	void setHasFinishedSending(bool value);
	void setConnectionType(ConnectionType connectionType);
	// We will not provide the setter for HTTPResponse as it should be managed by the HTTPResponse class
	/* Debugging */
	void printConnection() const;
};

#endif
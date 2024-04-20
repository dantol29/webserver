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

class Server; // Forward declaration for circular dependency

class Connection
{
  private:
	struct pollfd _pollFd;
	HTTPResponse _response;
	size_t _clientMaxHeadersSize;
	std::string _body;
	bool _bodyComplete;
	bool _bodyIsChunked;
	std::string _chunkData;

	// Additional client state can be managed here

  public:
	Connection(struct pollfd &pollFd, Server &server);
	Connection(const Connection &other);			// Copy constructor
	Connection &operator=(const Connection &other); // Copy assignment operator
	~Connection();

	bool readHeaders(Parser &parser);
	bool isChunked(Parser &parser);
	bool readChunkedBody();
	bool readChunkSize(std::string &line);
	bool readChunk(size_t chunkSize, std::string &chunkedData, HTTPResponse &response);
	bool readBody(Parser &parser);

	/* Getters */
	struct pollfd getPollFd() const;
	bool getBodyComplete() const;
	HTTPResponse &getResponse();
	std::string getBody() const;
	std::string getChunkData() const;
	bool getBodyIsChunked() const;
	/* Setters */
	void setHeadersComplete(bool headersComplete);
	void setBodyComplete(bool bodyComplete);
	void setBodyIsChunked(bool bodyIsChunked);
	void setHeaders(const std::string &headers);
	void setBody(const std::string &body);
	void setChunkData(const std::string &chunkData);
	// We will not provide the setter for HTTPResponse as it should be managed by the HTTPResponse class
	/* Debugging */
	void printConnection() const;
};

#endif
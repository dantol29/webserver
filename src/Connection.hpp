#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>	// for std::string and memset
#include <poll.h>	// For struct pollfd
#include <unistd.h> // For close
#include "Server.hpp"
#include "webserv.hpp"
#include "HTTPResponse.hpp" // Assuming existence of HTTPResponse class
#include "Parser.hpp"		// Assuming existence of Parser class

enum ConnectionType
{
	UNDEFINED,
	CLIENT,
	SERVER,
};

class Server; // Forward declaration for circular dependency

class Connection
{
  private:
	Parser _parser;
	HTTPRequest _request;
	HTTPResponse _response;

	struct pollfd _pollFd;
	enum ConnectionType _type;
	std::string _serverIp;
	unsigned short _serverPort;
	bool _hasReadSocket;
	bool _hasFinishedReading;
	bool _hasDataToSend;
	bool _hasFinishedSending;
	bool _canBeClosed;
	bool _hasCGI;
	bool _CGIexpired;
	int _PID;
	size_t _responseSize;
	size_t _responseSizeSent;
	std::string _responseString;

  public:
	Connection(struct pollfd &pollFd, Server &server);
	Connection(const Connection &other);
	Connection &operator=(const Connection &other);
	~Connection();

	bool readHeaders(Parser &parser);
	bool readChunkedBody(Parser &parser);
	bool readChunkSize(std::string &line);
	bool readChunk(size_t chunkSize, std::string &chunkedData, HTTPResponse &response);
	bool readBody(Parser &parser, HTTPRequest &req, HTTPResponse &res);

	/* Getters */
	Parser &getParser();
	HTTPRequest &getRequest();
	HTTPResponse &getResponse();

	struct pollfd getPollFd() const;

	std::string getResponseString() const;
	enum ConnectionType getType() const;
	std::string getServerIp() const;
	unsigned short getServerPort() const;
	size_t getResponseSize() const;
	size_t getResponseSizeSent() const;

	bool getHasReadSocket() const;
	bool getHasFinishedReading() const;
	bool getHasDataToSend() const;
	bool getHasFinishedSending() const;
	bool getCanBeClosed() const;

	/* CGI related */
	bool getHasCGI() const;
	bool getCGIexpired() const;
	int getPID() const;

	struct pollfd &getPollFd();

	/* Setters */
	void setResponseString(std::string responseString);
	void setType(enum ConnectionType type);
	void setServerIp(std::string serverIp);
	void setServerPort(unsigned short serverPort);
	void setResponseSize(size_t responseSize);
	void setResponseSizeSent(size_t responseSizeSent);

	void setHasReadSocket(bool value);
	void setHasFinishedReading(bool value);
	void setCanBeClosed(bool value);
	void setHasDataToSend(bool value);
	void setHasFinishedSending(bool value);

	/* CGI related */
	void setHasCGI(bool value);
	void setCGIexpired(bool value);
	void setPID(int PID);

	/* Debugging */
	void printConnection() const;
};

#endif
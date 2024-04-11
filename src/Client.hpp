#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>			// for std::string and memset
#include <poll.h>			// For struct pollfd
#include <unistd.h>			// For close
#include "HTTPResponse.hpp" // Assuming existence of HTTPResponse class

class Client
{
  private:
	struct pollfd _pollFd;
	HTTPResponse _response;
	std::string _headers;
	bool _headersComplete;
	size_t _headersTotalBytesRead;
	std::string _body;
	bool _bodyComplete;
	bool _bodyIsChunked;
	std::string _chunkData;

	// Additional client state can be managed here

  public:
	Client(struct pollfd pollFd);
	Client(const Client &other);			// Copy constructor
	Client &operator=(const Client &other); // Copy assignment operator
	~Client();

	bool readHeaders();
	bool readBody();
};

#endif
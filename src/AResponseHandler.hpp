#ifndef AResponseHandler_HPP
#define AResponseHandler_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "webserv.hpp"

class AResponseHandler
{

  public:
	AResponseHandler();
	virtual ~AResponseHandler();
	AResponseHandler(const AResponseHandler &other);
	AResponseHandler &operator=(const AResponseHandler &other);
	virtual HTTPResponse handleRequest(const HTTPRequest &request) = 0;
};

#endif
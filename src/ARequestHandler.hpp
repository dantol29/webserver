#ifndef AREQUESTHANDLER_HPP
#define AREQUESTHANDLER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "webserv.hpp"

class ARequestHandler
{

  public:
	ARequestHandler();
	virtual ~ARequestHandler();
	ARequestHandler(const ARequestHandler &other);
	ARequestHandler &operator=(const ARequestHandler &other);
	virtual HTTPResponse handleRequest(const HTTPRequest &request) = 0;
};

#endif
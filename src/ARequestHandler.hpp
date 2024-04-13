#ifndef AREQUESTHANDLER_HPP
#define AREQUESTHANDLER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "webserv.hpp"

class ARequestHandler
{

  public:
	ARequestHandler() {};
	virtual ~ARequestHandler()
	{
	}
	virtual HTTPResponse handleRequest(const HTTPRequest &request) = 0;
	virtual std::string handleCGIRequest(const HTTPRequest &request) = 0;

  private:
	ARequestHandler(const ARequestHandler &other);
	ARequestHandler &operator=(const ARequestHandler &other);
};

#endif
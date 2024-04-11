#ifndef AREQUESTHANDLER_HPP
#define AREQUESTHANDLER_HPP

#include "HTTPRequest.hpp"

class ARequestHandler
{

  public:
	ARequestHandler() {};
	virtual ~ARequestHandler()
	{
	}
	virtual std::string handleRequest(const HTTPRequest &request) = 0;

  private:
	ARequestHandler(const ARequestHandler &other);
	ARequestHandler &operator=(const ARequestHandler &other);
};

#endif
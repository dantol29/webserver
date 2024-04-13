#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "ARequestHandler.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Environment.hpp"

class CGIHandler : public ARequestHandler
{
  public:
	CGIHandler();
	virtual ~CGIHandler();
	virtual HTTPResponse handleRequest(const HTTPRequest &request);
	// std::string executeCGI(const HTTPRequest &request, const Environment &env);
	std::string executeCGI(const char *argv[], const Environment &env);

  private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
};

#endif // CGIHANDLER_HPP
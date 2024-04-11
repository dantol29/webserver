#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "ARequestHandler.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Environment.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

class CGIHandler : public ARequestHandler
{
public:
	CGIHandler();
	virtual ~CGIHandler();
	std::string handleRequest(const HTTPRequest &request);
	char** CGIHandler::createArgvForExecve(const Environment& env);
	std::string executeCGI(const Environment &env);

private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
};

#endif
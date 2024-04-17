#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "AResponseHandler.hpp"
#include "HTTPRequest.hpp"
#include "Environment.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>

class CGIHandler : public AResponseHandler
{
  public:
	CGIHandler();
	virtual ~CGIHandler();
	HTTPResponse handleRequest(const HTTPRequest &request);
	char *const *createArgvForExecve(const Environment &env);
	std::string executeCGI(const Environment &env);
	HTTPResponse CGIStringToResponse(const std::string &cgiOutput);

  private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
};

#endif
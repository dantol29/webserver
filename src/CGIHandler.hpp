#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "AResponseHandler.hpp"
#include "HTTPRequest.hpp"
#include "MetaVariables.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <cstring>

class CGIHandler : public AResponseHandler
{
  public:
	CGIHandler();
	virtual ~CGIHandler();
	void handleRequest(const HTTPRequest &request, HTTPResponse &response);
	void createArgvForExecve(const MetaVariables &env, std::vector<char *> &argv);
	std::string executeCGI(const MetaVariables &env);
	void CGIStringToResponse(const std::string &cgiOutput, HTTPResponse &response);

  private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
};

#endif
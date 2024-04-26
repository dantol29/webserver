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
	HTTPResponse handleRequest(const HTTPRequest &request);
	char *const *createArgvForExecve(const MetaVariables &env);
	std::string executeCGI(const MetaVariables &env);
	HTTPResponse CGIStringToResponse(const std::string &cgiOutput);
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	std::vector<struct pollfd> *getFDsRef();

  private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
	std::vector<pollfd> *_FDsRef;
};

#endif
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
#include <poll.h>

class CGIHandler : public AResponseHandler
{
  public:
	CGIHandler();
	virtual ~CGIHandler();
	void handleRequest(HTTPRequest &request, HTTPResponse &response);
	std::vector<std::string> createArgvForExecve(const MetaVariables &env);
	std::vector<char *> convertToCStringArray(const std::vector<std::string> &input);
	std::string executeCGI(const MetaVariables &env);
	void CGIStringToResponse(const std::string &cgiOutput, HTTPResponse &response);
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	void setPollFd(struct pollfd *pollFd);

  private:
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
	void closeAllSocketFDs();
	std::vector<pollfd> *_FDsRef;
	struct pollfd *_pollFd;
};

#endif
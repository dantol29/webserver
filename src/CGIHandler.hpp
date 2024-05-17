#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "EventManager.hpp"
#include "AResponseHandler.hpp"
#include "HTTPRequest.hpp"
#include "MetaVariables.hpp"
#include "Connection.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <poll.h>

class CGIHandler : public AResponseHandler
{
  public:
	CGIHandler(EventManager &eventManager, Connection &connection);
	CGIHandler(const CGIHandler &other);
	CGIHandler &operator=(const CGIHandler &other);
	virtual ~CGIHandler();
	void handleRequest(const HTTPRequest &request, HTTPResponse &response);
	bool executeCGI(const MetaVariables &env, HTTPResponse &response);
	std::vector<std::string> createArgvForExecve(const MetaVariables &env);
	std::vector<char *> convertToCStringArray(const std::vector<std::string> &input);
	// void CGIStringToResponse(const std::string &cgiOutput, HTTPResponse &response);
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	void setPollFd(struct pollfd *pollFd);

  private:
	void closeAllSocketFDs();
	std::vector<pollfd> *_FDsRef;
	struct pollfd *_pollFd;
	Connection &_connection;
	EventManager &_eventManager;
};

#endif
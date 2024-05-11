#ifndef STATICCONTENTHANDLER_HPP
#define STATICCONTENTHANDLER_HPP

#include "AResponseHandler.hpp"
#include <string>
#include <fstream>
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <sstream>
#include "webserv.hpp"

class StaticContentHandler : public AResponseHandler
{
  public:
	StaticContentHandler();
	StaticContentHandler(const std::string &webRoot);
	~StaticContentHandler();
	void handleRequest(const HTTPRequest &request, HTTPResponse &response);

	// std::string handleNotFound();  or setErrorResponse(int statusCode) from WEB-127
	void handleNotFound(HTTPResponse &response);

  private:
	std::string _webRoot;
	StaticContentHandler(const StaticContentHandler &other);
	StaticContentHandler &operator=(const StaticContentHandler &other);
};

#endif
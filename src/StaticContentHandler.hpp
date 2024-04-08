#ifndef STATICCONTENTHANDLER_HPP
#define STATICCONTENTHANDLER_HPP

#include "ARequestHandler.hpp"
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

class StaticContentHandler : public ARequestHandler
{
  public:
	StaticContentHandler(const std::string &webRoot);
	~StaticContentHandler();
	HTTPResponse handleRequest(const HTTPRequest &request);

	std::string handleHomePage();
	std::string handleNotFound();

  private:
	StaticContentHandler();
	StaticContentHandler(const StaticContentHandler &other);
	StaticContentHandler &operator=(const StaticContentHandler &other);

	std::string _webRoot;
};

#endif // STATICCONTENTHANDLER_HPP
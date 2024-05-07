#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include "AResponseHandler.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include "webserv.hpp"

class UploadHandler : public AResponseHandler
{
  public:
	UploadHandler();
	UploadHandler(const std::string &webRoot);
	~UploadHandler();
	void handleRequest(const HTTPRequest &request, HTTPResponse &response);
	void handleResponse(HTTPResponse &response, const std::string &code);

  private:
	std::string _webRoot;
	UploadHandler(const UploadHandler &other);
	UploadHandler &operator=(const UploadHandler &other);
};

#endif
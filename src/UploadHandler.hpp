#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include "AResponseHandler.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "webserv.hpp"

class UploadHandler : public AResponseHandler
{
  public:
	UploadHandler();
	UploadHandler(const std::string &webRoot);
	UploadHandler &operator=(const UploadHandler &other);
	void handleRequest(const HTTPRequest &request, HTTPResponse &response);
	~UploadHandler();

  private:
	std::string _webRoot;
	UploadHandler(const UploadHandler &other);
	void handleResponse(HTTPResponse &response, const std::string &code);
	bool isHarmfulExtension(const std::string &extension);
	void checkFiles(const HTTPRequest &request);
};

#endif
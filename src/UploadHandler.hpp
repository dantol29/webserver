#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include "AResponseHandler.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "webserv.hpp"

enum UploadStatus
{
	SUCCESS,
	BAD_REQUEST,
	INTERNAL_SERVER_ERROR,
};

class UploadHandler : public AResponseHandler
{
  public:
	UploadHandler();
	UploadHandler(const std::string &webRoot);
	UploadHandler &operator=(const UploadHandler &other);
	void handleRequest(HTTPRequest &request, HTTPResponse &response);
	~UploadHandler();
	void setUploadDir(const std::string &uploadDir);

  private:
    std::string _uploadDir;
	std::string _webRoot;
	UploadHandler(const UploadHandler &other);
	void handleResponse(HTTPResponse &response, enum UploadStatus status);
	std::string readFileContents(const std::string &filePath);
	bool isHarmfulExtension(const std::string &extension);
	bool checkFiles(const HTTPRequest &request);
	bool createFile(HTTPRequest &request);
	bool createFileChunked(HTTPRequest &request);
};

#endif
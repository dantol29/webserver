#ifndef PARSER_HPP
#define PARSER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#define MAX_URI 200

//./client $'POST /search?price=low HTTP/1.1\r\nHost: www.example.com\r\nContent-type: text/plain\r\nContent-length:
// 42\r\n\r\n7\r\nChunk 1\r\n6\r\nChunk 2\r\n0\r\n\r\n'
// test line

class Parser
{
  public:
	~Parser();
	Parser();
	void parseRequest(const char *request, HTTPRequest &req, HTTPResponse &res);

  private:

	// PARSING FUNC
	void parseRequestLine(const char *request, HTTPRequest &req, HTTPResponse &res);
	void parseHeaders(const char *request, HTTPRequest &req, HTTPResponse &res);
	void parseBody(const char *request, HTTPRequest &req, HTTPResponse &res);
	void parseFileBody(const char *request, HTTPRequest &req, HTTPResponse &res);

	// UTILS
	bool saveFileHeaders(const std::string& headers, HTTPRequest& req, unsigned int& i);
	bool saveFileData(const std::string& data, HTTPRequest& req, unsigned int& i, bool& isFinish);
	bool isUploadBoundary(const std::string& data, HTTPRequest &req, unsigned int& i);
	std::string extractUploadBoundary(std::string line);
	std::string extractVariables(std::string &requestTarget, bool &isOriginForm);
	bool saveVariables(std::string &variables, HTTPRequest &req);
	bool isValidHost(std::string host);
	bool isValidContentType(std::string type);
	bool isOrigForm(std::string &requestTarget, int &queryStart);
	void skipRequestLine(const char *request, unsigned int &i);
	void skipHeader(const char *request, unsigned int &i);
	bool hasMandatoryHeaders(HTTPRequest &obj);
	std::string extractValue(std::string &variables, int &i);
	std::string extractKey(std::string &variables, int &i, int startPos);
	std::string extractRequestTarget(const char *request, unsigned int &i);
	std::string extractProtocolVersion(const char *request, unsigned int &i);
	std::string extractMethod(const char *request, unsigned int &i);
	std::string extractHeaderKey(const char *request, unsigned int &i);
	std::string extractHeaderValue(const char *request, unsigned int &i);
	unsigned int extractLineLength(const char *request, unsigned int &i);
	std::string extractLine(const char *request, unsigned int &i, const unsigned int &size);
	
	// CHUNKED REQUESTS
	// Maybe not needed
	bool getIsChunked() const;
	bool getIsChunkFinish() const;
	void setIsChunked(bool a);
	void parseChunkedBody(const char *request, HTTPRequest &req, HTTPResponse &res);
	bool _isChunked;
	bool _isChunkFinish;
};

#endif
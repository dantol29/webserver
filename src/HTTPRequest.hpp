#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <iostream>
#include <unistd.h>
#include <map>
#include <vector>

// For file upload
// ./client $'POST / HTTP/1.1\r\nhost: www.example.com\r\ncontent-type: multipart/form-data; boundary=1234567890\r\ncontent-length: 5\r\n\r\n1234567890\r\nContent-Disposition: form-data; name: text\r\n\r\nSome text data\r\nMore data\r\nOK\r\n1234567890\r\nheader1: value1; header2: value2\r\n\r\nhahaha\r\n1234567890--\r\n\r\n'

// It is RECOMMENDED that all HTTP
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
#define MAX_URI 200

struct File;

class HTTPRequest
{
  public:
	HTTPRequest(const HTTPRequest &obj);
	HTTPRequest &operator=(const HTTPRequest &obj);
	~HTTPRequest();
	HTTPRequest(const char *request);

	// GETTERS
	int getStatusCode() const;
	std::string getMethod() const;
	std::string getHost() const;
	std::string getRequestTarget() const;
	std::string getProtocolVersion() const;
	std::string getErrorMessage() const;
	std::string getUploadBoundary() const;
	bool getIsChunked() const;
	bool getIsChunkFinish() const;
	std::multimap<std::string, std::string> getQueryString() const;
	std::multimap<std::string, std::string> getHeaders() const;
	std::pair<std::string, std::string> getHeaders(std::string key) const;
	std::vector<std::string> getBody() const;
	std::vector<File> getFiles() const;

	// CHUNKED REQUESTS
	void setIsChunked(bool a);
	bool parseChunkedBody(const char *request);

  private:
	
	// INTERNAL TOOLS
	HTTPRequest();
	bool parseRequestLine(const char *request);
	bool parseHeaders(const char *request);
	bool parseBody(const char *request);
	bool parseFileBody(const char *request);
	bool ft_error(int statusCode, std::string message);

	// FILE UPLOAD
	struct File
	{
		std::map<std::string, std::string> headers;
		std::vector<std::string> fileContent;
	};

	// VARIABLES
	int _statusCode;
	bool _isChunked;
	bool _isChunkFinish;
	std::string _method;
	std::string _requestTarget;
	std::string _protocolVersion;
	std::string	_uploadBoundary;
	std::string _errorMessage;
	std::multimap<std::string, std::string> _queryString;
	std::multimap<std::string, std::string> _headers;
	std::vector<std::string> _body;
	std::vector<File> _files;

	// UTILS
	bool isUploadBoundary(const std::string& data, unsigned int& i);
	bool saveFileHeaders(const std::string& headers, unsigned int& i);
	bool saveFileData(const std::string& data, unsigned int& i, bool& isFinish);
	bool saveVariables(std::string &variables);
	void makeHeadersLowCase();
	bool isValidHost(std::string host);
	bool isValidContentType(std::string type);
	bool isOrigForm(std::string &requestTarget, int &queryStart);
	void skipRequestLine(const char *request, unsigned int &i);
	void skipHeader(const char *request, unsigned int &i);
	bool hasMandatoryHeaders();
	unsigned int extractLineLength(const char *request, unsigned int &i);
	std::string extractValue(std::string &variables, int &i);
	std::string extractKey(std::string &variables, int &i, int startPos);
	std::string extractRequestTarget(const char *request, unsigned int &i);
	std::string extractVariables(std::string &requestTarget, bool &isOriginForm);
	std::string extractProtocolVersion(const char *request, unsigned int &i);
	std::string extractMethod(const char *request, unsigned int &i);
	std::string extractHeaderKey(const char *request, unsigned int &i);
	std::string extractHeaderValue(const char *request, unsigned int &i);
	std::string extractLine(const char *request, unsigned int &i, const unsigned int &size);
	std::string extractUploadBoundary(std::string line);
};

std::ostream& operator<<(std::ostream& out, const HTTPRequest& obj);

#endif
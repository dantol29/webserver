#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <iostream>
#include <unistd.h>
#include <map>
#include <vector>

// It is RECOMMENDED that all HTTP
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
#define MAX_URI 200

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

	// CHUNKED REQUESTS
	void setIsChunked(bool a);
	bool parseChunkedBody(const char *request);

	// FILE UPLOAD
	struct File
	{
		std::map<std::string, std::string> headers;
		std::vector<std::string> fileContent;
	};


  private:
	HTTPRequest();
	bool parseRequestLine(const char *request);
	bool parseHeaders(const char *request);
	bool parseBody(const char *request);
	bool parseFileBody(const char *request);
	bool ft_error(int statusCode, std::string message);

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
	bool saveFileHeaders(std::string& headers);
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
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <iostream>
#include <unistd.h>
#include <map>

// It is RECOMMENDED that all HTTP
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
#define MAX_URI 200

// #include <string.h>
// #include <stdio.h>
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;
// insert after read in the main to test

class HTTPRequest
{
  public:
	HTTPRequest();
	HTTPRequest(const char *request);
	~HTTPRequest();
	std::string getMethod() const;
	int getStatusCode() const;
	std::string getRequestTarget() const;
	std::string getProtocolVersion() const;

  private:
	int parseRequestLine(char *request);
	int _statusCode;
	std::string _method;
	std::string _requestTarget;
	std::string _protocolVersion;
};

bool isOrigForm(std::string &requestTarget, int &queryStart);
bool fileExists(std::string &requestTarget, bool isOriginForm, int queryStart);
std::string extractValue(std::string &variables, int &i);
std::string extractKey(std::string &variables, int &i, int startPos);
std::string extractRequestTarget(char *request, int &i);
std::string extractProtocolVersion(char *request, int &i);
std::string extractMethod(char *request, int &i);

#endif
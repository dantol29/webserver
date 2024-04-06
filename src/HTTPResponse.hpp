#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <iostream>
#include <vector>

class HTTPResponse
{
  public:
	// Default constructor
	HTTPResponse();
	// Copy constructor
	HTTPResponse(const HTTPResponse &other);
	// Assignment operator
	HTTPResponse &operator=(const HTTPResponse &other);
	// Destructor
	~HTTPResponse();

	void setStatusCode(int statusCode);
	void setHeader(const std::string &name, const std::string &value);
	void setBody(const std::string &body);
	std::string toString() const;

  private:
	int _statusCode;
	std::vector<std::pair<std::string, std::string>> _headers;
	std::string _body;

	std::string getStatusMessage(int statusCode) const;
};

#endif // HTTPRESPONSE_HPP
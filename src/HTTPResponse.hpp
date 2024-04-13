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

	int getStatusCode() const;
	const std::string &getStatusMessage() const;
	void setStatusCode(int statusCode);
	std::string getBody() const;

	void setHeader(const std::string &name, const std::string &value);
	void setBody(const std::string &body);
	std::string toString() const;

  private:
	int _statusCode;
	std::string _statusMessage;
	// clang-format off
	std::vector<std::pair<std::string, std::string> > _headers;
	// clang-format on
	std::string _body;
	// private cause it's used only to set the status message based on the status code
	std::string getStatusMessage(int statusCode) const;
};

#endif // HTTPRESPONSE_HPP
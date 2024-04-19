#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <iostream>
#include <vector>
#include "webserv.hpp"

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
	std::string getBody() const;
	std::string getHeader(const std::string &name) const;
	void setErrorResponse(int statusCode);
	std::string objToString() const;

	void setStatusCode(int statusCode);
	void setHeader(const std::string &name, const std::string &value);
	void setBody(const std::string &body);

	bool isCGI() const;
	void setIsCGI(bool isCGI);

	friend std::ostream &operator<<(std::ostream &out, const HTTPResponse &response);

  private:
	int _statusCode;
	std::string _statusMessage;
	// clang-format off
	std::vector<std::pair<std::string, std::string> > _headers;
	// clang-format on
	std::string _body;
	// private cause it's used only to set the status message based on the status code
	std::string getStatusMessage(int statusCode) const;
	bool _isCGI;
};

#endif // HTTPRESPONSE_HPP
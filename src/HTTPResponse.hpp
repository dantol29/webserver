#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <iostream>
#include <vector>
#include "webserv.hpp"

class HTTPResponse
{
  public:
	HTTPResponse();
	HTTPResponse(const HTTPResponse &other);
	HTTPResponse &operator=(const HTTPResponse &other);
	~HTTPResponse();

	int getStatusCode() const;
	const std::string &getStatusMessage() const;
	std::string getBody() const;
	std::string getHeader(const std::string &name) const;
	void setErrorResponse(int statusCode);
	std::string objToString() const;

	void setStatusCode(int statusCode, const std::string &message);
	void setHeader(const std::string &name, const std::string &value);
	void setBody(const std::string &body);

	bool getIsCGI() const;
	void setIsCGI(bool value);

	// Easy version
	// int *getCGIpipeFD();
	// void setCGIpipeFD(int *CGIpipeFD);
	// Fancy version
	// int (*getCGIpipeFD())[2];
	// void setCGIpipeFD(int (*CGIpipeFD)[2]);
	// Fancy version with reference
	int (&getCGIpipeFD())[2];
	void setCGIpipeFD(int (&pipe)[2]);
	// arrays are passed by reference by default, but returning array in C++ is not alowed
	// int(getCGIpipeFD())[2];
	// void setCGIpipeFD(int CGIpipeFD[2]);

	// Member functions
	void CGIStringToResponse(const std::string &cgiOutput);

	friend std::ostream &operator<<(std::ostream &out, const HTTPResponse &response);

  private:
	int _statusCode;
	std::string _statusMessage;
	// clang-format off
	std::vector<std::pair<std::string, std::string> > _headers;
	// clang-format on
	std::string _body;
	bool _isCGI;
	int _CGIpipeFD[2];
	// private cause it's used only to set the status message based on the status code
	std::string getStatusMessage(int statusCode) const;
};

#endif // HTTPRESPONSE_HPP
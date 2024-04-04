#include <string>
#include <iostream>
#include <unistd.h>

# define MAX_URI 200


// #include <string.h>
// #include <stdio.h>
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;
// insert after read in the main to test

class HTTPRequest{
	public:
		HTTPRequest();
		HTTPRequest(char *request);
		~HTTPRequest();
		std::string getMethod() const;
		int			getStatusCode() const;
		std::string getRequestTarget() const;
		std::string getProtocolVersion() const;
	private:
		int			parseRequestLine(char *request);
		int			_statusCode;
		std::string _method;
		std::string	_requestTarget;
		std::string _protocolVersion;
};

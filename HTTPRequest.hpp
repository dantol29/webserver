#include <string>
#include <iostream>
#include <unistd.h>
#include <map>

// It is RECOMMENDED that all HTTP 
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
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
		HTTPRequest(const HTTPRequest& obj);
		HTTPRequest& operator=(const HTTPRequest& obj);
		~HTTPRequest();
		HTTPRequest(char *request);
		std::string getMethod() const;
		int			getStatusCode() const;
		std::string getRequestTarget() const;
		std::string getProtocolVersion() const;
		std::map<std::string, std::string>	getStorage() const;
		bool		addStorage(std::string key, std::string value);
	private:
		int			parseRequestLine(char *request);
		int			_statusCode;
		std::map<std::string, std::string> _storage;		
		std::string _method;
		std::string	_requestTarget;
		std::string _protocolVersion;
};

bool isOrForm(std::string requestTarget, int &queryStart);
bool fileExists(std::string requestTarget, bool isOriginForm, int queryStart);
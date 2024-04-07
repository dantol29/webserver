#include <string>
#include <iostream>
#include <unistd.h>
#include <map>

// It is RECOMMENDED that all HTTP 
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
# define MAX_URI 200

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
		bool		getIsChunked() const;
		std::multimap<std::string, std::string>	getStorage() const;
		std::multimap<std::string, std::string>	getHeaders() const;
		std::pair<std::string, std::string>		getHeaders(std::string key) const;
		void		addStorage(std::string key, std::string value);
		void		addHeader(std::string key, std::string value);
		void		setIsChunked(bool a);
	private:
		int			parseRequestLine(char *request);
		int			_statusCode;
		bool		_isChunked;
		std::string _method;
		std::string	_requestTarget;
		std::string _protocolVersion;
		std::multimap<std::string, std::string> _storage;
		std::multimap<std::string, std::string> _headers;	
};

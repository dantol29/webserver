#include <string>
#include <iostream>
#include <unistd.h>
#include <map>
#include <vector>

// It is RECOMMENDED that all HTTP 
// senders and recipients support, at a minimum, request-line lengths of 8000 octets
# define MAX_URI 200

//./client $'POST /search?price=low HTTP/1.1\r\nHost: www.example.com\r\nContent-type: text/plain\r\nContent-length: 42\r\n\r\n7\r\nChunk 1\r\n6\r\nChunk 2\r\n0\r\n\r\n'
// test line

class HTTPRequest{
	public:
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
		std::vector<std::string>				getBody() const;
		void		addStorage(std::string key, std::string value);
		void		addHeader(std::string key, std::string value);
		void		addToBody(char *request);
		void		setIsChunked(bool a);
		int			parseChunkedBody(char *request);
	private:
		HTTPRequest();
		int			parseRequestLine(char *request);
		int			_statusCode;
		bool		_isChunked;
		std::string _method;
		std::string	_requestTarget;
		std::string _protocolVersion;
		std::vector<std::string>				_body;
		std::multimap<std::string, std::string> _storage;
		std::multimap<std::string, std::string> _headers;	
};

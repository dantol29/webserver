#include "HTTPRequest.hpp"
#include <string.h>

bool		isOrigForm(std::string& requestTarget, int &queryStart);
bool		fileExists(std::string& requestTarget, bool isOriginForm, int queryStart);
void		skipRequestLine(const char *request, unsigned int& i);
void		skipHeader(const char *request, unsigned int& i);
bool		hasMandatoryHeaders(HTTPRequest& obj);
bool		hasCRLF(const char* request, unsigned int& i, int mode);
std::string		extractValue(std::string& variables, int &i);
std::string 	extractKey(std::string& variables, int &i, int startPos);
std::string 	extractRequestTarget(const char *request, unsigned int& i);
std::string		extractVariables(std::string& requestTarget, bool& isOriginForm);
std::string 	extractProtocolVersion(const char *request, unsigned int& i);
std::string		extractMethod(const char *request, unsigned int& i);
std::string		extractHeaderKey(const char *request, unsigned int& i);
std::string		extractHeaderValue(const char *request, unsigned int& i);
unsigned int	extractLineLength(const char *request, unsigned int& i);
std::string		extractLine(const char *request, unsigned int& i, const unsigned int& size);
int			parseBody();

HTTPRequest::HTTPRequest() : _statusCode(200), _isChunked(false), _method(""), \
_requestTarget(""), _protocolVersion(""){

}

HTTPRequest::HTTPRequest(const HTTPRequest& obj){
	_statusCode = obj._statusCode;
	_isChunked = obj._isChunked;
	_method = obj._method;
	_requestTarget = obj._requestTarget;
	_protocolVersion = obj._protocolVersion;
	_body = obj._body;
	_headers  = obj._headers;
	_queryString = obj._queryString;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& obj){
	if (this != &obj){
		_statusCode = obj._statusCode;
		_isChunked = obj._isChunked;
		_method = obj._method;
		_requestTarget = obj._requestTarget;
		_protocolVersion = obj._protocolVersion;
		_body = obj._body;
		_headers  = obj._headers;
		_queryString = obj._queryString;
	}
	return (*this);
}

HTTPRequest::~HTTPRequest(){

}

HTTPRequest::HTTPRequest(char *request){
	if (strlen(request) < 10)
		_statusCode = 400;
	else{
		_statusCode = parseRequestLine(request);
		if (_statusCode == 200)
			_statusCode = parseHeaders(request);
		else if (_statusCode == 200 && !_isChunked)
			parseBody();// parse regular body
	}
}

std::string HTTPRequest::getMethod() const{
	return (_method);
}

std::string HTTPRequest::getProtocolVersion() const{
	return (_protocolVersion);
}

std::string HTTPRequest::getRequestTarget() const{
	return (_requestTarget);
}

int	HTTPRequest::getStatusCode() const{
	return (_statusCode);
}

bool	HTTPRequest::getIsChunked() const{
	return (_isChunked);
}

std::multimap<std::string, std::string>	HTTPRequest::getQueryString() const{
	return (_queryString);
}

std::multimap<std::string, std::string>	HTTPRequest::getHeaders() const{
	return (_headers);
}

std::pair<std::string, std::string> HTTPRequest::getHeaders(std::string key) const{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it){
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

bool HTTPRequest::getIsChunkFinish() const{
	return (_isChunkFinish);
}

std::vector<std::string> HTTPRequest::getBody() const{
	return (_body);
}

void	HTTPRequest::setIsChunked(bool n){
	_isChunked = n;
}

bool	HTTPRequest::saveVariables(std::string& variables)
{
	int	startPos = 0;
	std::string	key;
	std::string	value;

	for (int i = 0; i < (int)variables.length(); i++){
		if (variables[i] == '='){
			key = extractKey(variables, i, startPos);
			if (key.empty())
				return (false);
			value = extractValue(variables, i);
			if (value.empty())
				return (false);
			startPos = i;
			_queryString.insert(std::make_pair(key, value));
		}
	}
	return (true);
}

int	HTTPRequest::parseRequestLine(const char *request)
{
	unsigned int	i = 0;
	bool			isOriginForm = false;
	std::string		variables;

	_method = extractMethod(request, i);
	if (_method.empty()) // A server that receives a method longer than any that it implements
		return (501); // SHOULD respond with a 501 (Not Implemented).
	if (request[i++] != ' ') // single space
		return (400);
	_requestTarget = extractRequestTarget(request, i);
	if (_requestTarget.empty()) // A server that receives a request-target longer 
		return (414); // than any URI it wishes to parse MUST respond with a 414 (URI Too Long).
	if (request[i++] != ' ') // single space
		return (400);
	variables = extractVariables(_requestTarget, isOriginForm);
	if (variables.empty())
		return (400);
	if (isOriginForm)
		if (!saveVariables(variables))
			return (false);
	_protocolVersion = extractProtocolVersion(request, i);
	if (_protocolVersion.empty())
		return (400);
	if (!hasCRLF(request, i, 0))
		return (400);
	return (200); 
}

int	HTTPRequest::parseChunkedBody(const char *request)
{
	unsigned int	i = 0;
	int				size = 0;
	std::string		line;

	skipHeader(request, i);
	while (request[i]){
		size = extractLineLength(request, i);
		if (size <= 0){
			if (size == -1)
				return (400);
			break ;
		}
		line = extractLine(request, i, size);
		if (line.empty())
			return (400);
		_body.push_back(line);
	}
	if (!hasCRLF(request, i, 0))
		return (400);
	if (hasCRLF(request, i, 1))
		_isChunkFinish = true;
	return (200);
}

int HTTPRequest::parseHeaders(const char *request)
{
	unsigned int	i;
	std::string		key;
	std::string		value;

	i = 0;
	skipRequestLine(request, i);
	while (request[i]){
		key = extractHeaderKey(request, i);
		if (key.empty())
			return (400);
		i++; // skip ':'
		if (request[i++] != ' ')
			return (400);
		value = extractHeaderValue(request, i);
		if (value.empty())
			return (400);
		if (request[i] != '\r' || request[i + 1] != '\n')
			return (400);
		_headers.insert(std::make_pair(key, value));
		i += 2; // skip '\r' and '\n'
		if (request[i] == '\r' && request[i + 1] == '\n') // end of header section
			break ;
	}
	if (request[i] != '\r' || request[i + 1] != '\n') // end of header section
		return (400);
	if (!hasMandatoryHeaders(*this))
		return (400);
	if (_method == "GET" && request[i + 2]) //has something after headers
		return (400);
	return (200);
}

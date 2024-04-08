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
std::string 	extractProtocolVersion(const char *request, unsigned int& i);
std::string		extractMethod(const char *request, unsigned int& i);
std::string		extractHeaderKey(const char *request, unsigned int& i);
std::string		extractHeaderValue(const char *request, unsigned int& i);
unsigned int	extractLineLength(const char *request, unsigned int& i);
std::string		extractLine(const char *request, unsigned int& i, const unsigned int& size);
int 		parseHeaders(const char *request, HTTPRequest& obj);
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
	_storage = obj._storage;
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
		_storage = obj._storage;
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
			_statusCode = parseHeaders(request, *this);
		if (_statusCode == 200 && _isChunked)
			_statusCode = parseChunkedBody(request);// parse chunked body
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

std::multimap<std::string, std::string>	HTTPRequest::getStorage() const{
	return (_storage);
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

void	HTTPRequest::addStorage(std::string key, std::string value){
	_storage.insert(std::make_pair(key, value));
}

void	HTTPRequest::addHeader(std::string key, std::string value){
	_headers.insert(std::make_pair(key, value));
}

void	HTTPRequest::setIsChunked(bool n){
	_isChunked = n;
}

bool	saveVariables(std::string variables, HTTPRequest* obj)
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
			obj->addStorage(key, value);
		}
	}
	return (true);
}

bool	checkRequestTarget(std::string& requestTarget, HTTPRequest* obj)
{
	bool	isOriginForm;
	int		queryStart = 0;

	if (requestTarget == "/")
		return (true);
	isOriginForm = isOrigForm(requestTarget, queryStart);
	if (!fileExists(requestTarget, isOriginForm, queryStart))
		return (false);
	if (isOriginForm)
		if (!saveVariables(requestTarget.substr(\
		queryStart + 1, strlen(requestTarget.c_str()) - queryStart), obj))
			return (false);
	return (true);
}

int	HTTPRequest::parseRequestLine(char *request)
{
	unsigned int	i = 0;

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
	if (!checkRequestTarget(_requestTarget, this))
		return (400);
	_protocolVersion = extractProtocolVersion(request, i);
	if (_protocolVersion.empty())
		return (400);
	if (!hasCRLF(request, i, 0))
		return (400);
	return (200); 
}

int	HTTPRequest::parseChunkedBody(char *request)
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
	if (!hasCRLF(request, i, 1))
		return (400);
	return (200);
}

void	HTTPRequest::addToBody(char *request)
{
	(void)request;
	// add more text to the body
}

std::vector<std::string> HTTPRequest::getBody() const{
	return (_body);
}
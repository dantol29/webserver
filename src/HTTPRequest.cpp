#include "HTTPRequest.hpp"
#include <string.h>

bool		isOrigForm(std::string& requestTarget, int &queryStart);
bool		fileExists(std::string& requestTarget, bool isOriginForm, int queryStart);
void		skipRequestLine(char *request, int& i);
bool		hasMandatoryHeaders(HTTPRequest& obj);
std::string	extractValue(std::string& variables, int &i);
std::string extractKey(std::string& variables, int &i, int startPos);
std::string extractRequestTarget(char *request, int &i);
std::string extractProtocolVersion(char *request, int &i);
std::string	extractMethod(char *request, int &i);
std::string	extractHeaderKey(char *request, int& i);
std::string	extractHeaderValue(char *request, int& i);
int 		parseHeaders(char *request, HTTPRequest& obj);
int			parseBody();
int			parseChunkedBody();

HTTPRequest::HTTPRequest() : _statusCode(200), _isChunked(false), _method(""), \
_requestTarget(""), _protocolVersion(""), _body(""){

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
			parseChunkedBody(request);// parse chunked body
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
	int			i;

	i = 0;
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
		return (400); // An invalid request-line SHOULD respond with a 400 (Bad Request).
	_protocolVersion = extractProtocolVersion(request, i);
	if (_protocolVersion.empty())
		return (400);
	if (request[i] != '\r' || !request[i + 1] || request[i + 1] != '\n') // CRLF
		return (400); // The combination of \r\n serves as a standard way to denote the end of a line in HTTP headers.
	return (200); 
}

bool	HTTPRequest::parseChunkedBody(char *request)
{
	// parse the whole body
	(void)request;
	return (true);
}

void	HTTPRequest::addToBody(char *request)
{
	(void)request;
	// add more text to the body
}

std::string HTTPRequest::getBody() const{
	return (_body);
}
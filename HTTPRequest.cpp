#include "HTTPRequest.hpp"
#include <string.h>

HTTPRequest::HTTPRequest() : _method(""), _requestTarget(""), _protocolVersion(""){

}

HTTPRequest::HTTPRequest(const HTTPRequest& obj){
	_method = obj._method;
	_requestTarget = obj._requestTarget;
	_protocolVersion = obj._protocolVersion;
	_statusCode = obj._statusCode;
	_storage = obj._storage;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& obj){
	if (this != &obj){
		_method = obj._method;
		_requestTarget = obj._requestTarget;
		_protocolVersion = obj._protocolVersion;
		_statusCode = obj._statusCode;
		_storage = obj._storage;
	}
	return (*this);
}

HTTPRequest::~HTTPRequest(){

}

HTTPRequest::HTTPRequest(char *request){
	_statusCode = parseRequestLine(request);
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

std::map<std::string, std::string>	HTTPRequest::getStorage() const{
	return (_storage);
}

bool	HTTPRequest::addStorage(std::string key, std::string value){
	_storage[key] = value;
	return (true);
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

bool	checkRequestTarget(std::string requestTarget, HTTPRequest* obj)
{
	bool	isOriginForm;
	int		queryStart = 0;

	if (requestTarget == "/")
		return (true);
	isOriginForm = isOrigForm(requestTarget, queryStart);
	if (!fileExists(requestTarget, isOriginForm, queryStart))
		return (false);
	if (isOriginForm)
		saveVariables(requestTarget.substr(\
		queryStart + 1, strlen(requestTarget.c_str()) - queryStart), obj);
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

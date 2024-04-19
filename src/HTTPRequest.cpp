#include "HTTPRequest.hpp"
#include "webserv.hpp"

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::~HTTPRequest()
{
}

std::string HTTPRequest::getMethod() const
{
	return (_method);
}

// TODO: Check if "Host" should be written in lower case
std::string HTTPRequest::getHost() const
{
	std::multimap<std::string, std::string>::const_iterator it = _headers.find("Host");
	if (it != _headers.end())
	{
		return it->second;
	}
	return "";
}

std::string HTTPRequest::getRequestTarget() const
{
	return (_requestTarget);
}

std::string HTTPRequest::getProtocolVersion() const
{
	return (_protocolVersion);
}

std::multimap<std::string, std::string> HTTPRequest::getQueryString() const
{
	return (_queryString);
}

std::multimap<std::string, std::string> HTTPRequest::getHeaders() const
{
	return (_headers);
}

std::pair<std::string, std::string> HTTPRequest::getSingleHeader(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

std::vector<std::string> HTTPRequest::getBody() const
{
	return (_body);
}

void HTTPRequest::setMethod(std::string method)
{
	_method = method;
}

void HTTPRequest::setRequestTarget(std::string requestTarget)
{
	_requestTarget = requestTarget;
}

void HTTPRequest::setQueryString(const std::string &key, const std::string &value)
{
	_queryString.insert(std::make_pair(key, value));
}

// void HTTPRequest::setHeaders(const std::string &key, const std::string &value)
// {
// 	_headers.insert(std::make_pair(key, value));
// }

// This makes the key lowercase and then inserts the original key
void HTTPRequest::setHeaders(const std::string &key, const std::string &value)
{
	std::string lowerKey = key;
	for (size_t i = 0; i < lowerKey.size(); ++i)
		lowerKey[i] = std::tolower(static_cast<unsigned char>(lowerKey[i]));
	_headers.insert(std::make_pair(lowerKey, value));
}

void HTTPRequest::setBody(const std::string &body)
{
	_body.push_back(body);
}

void HTTPRequest::setProtocolVersion(std::string protocolVersion)
{
	_protocolVersion = protocolVersion;
}

std::ostream &operator<<(std::ostream &out, const HTTPRequest &obj)
{
	std::multimap<std::string, std::string> headers = obj.getHeaders();
	std::multimap<std::string, std::string> queryString = obj.getQueryString();
	std::vector<std::string> body = obj.getBody();

	std::multimap<std::string, std::string>::iterator it;
	out << "---------------------Variables--------------------" << std::endl;
	for (it = queryString.begin(); it != queryString.end(); it++)
	{
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	out << "---------------------End--------------------------" << std::endl;
	out << "---------------------Headers----------------------" << std::endl;
	for (it = headers.begin(); it != headers.end(); it++)
	{
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	out << "---------------------End--------------------------" << std::endl;
	out << "---------------------Body-------------------------" << std::endl;
	for (size_t i = 0; i < body.size(); ++i)
		out << body[i] << std::endl;
	out << "---------------------End--------------------------" << std::endl;
	return (out);
}

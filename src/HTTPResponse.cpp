#include "HTTPResponse.hpp"
#include <sstream>

HTTPResponse::HTTPResponse() : _statusCode(200)
{
}

HTTPResponse::HTTPResponse(const HTTPResponse &other)
	: _statusCode(other._statusCode), _headers(other._headers), _body(other._body)
{
}

HTTPResponse &HTTPResponse::operator=(const HTTPResponse &other)
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::setStatusCode(int statusCode)
{
	_statusCode = statusCode;
}

void HTTPResponse::setHeader(const std::string &name, const std::string &value)
{
	_headers.push_back(std::make_pair(name, value));
}

void HTTPResponse::setBody(const std::string &body)
{
	_body = body;
	setHeader("Content-Length", std::to_string(body.size()));
}

std::string HTTPResponse::toString() const
{
	std::stringstream responseStream;
	responseStream << "HTTP/1.1 " << _statusCode << " " << getStatusMessage(_statusCode) << "\r\n";
	for (size_t i = 0; i < _headers.size(); ++i)
	{
		responseStream << _headers[i].first << ": " << _headers[i].second << "\r\n";
	}

	responseStream << "\r\n";
	responseStream << _body;
	return responseStream.str();
}

std::string HTTPResponse::getStatusMessage(int statusCode) const
{
	switch (statusCode)
	{
	case 200:
		return "OK";
	case 404:
		return "Not Found";
	case 500:
		return "Internal Server Error";
	default:
		return "Unknown";
	}
}
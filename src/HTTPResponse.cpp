#include "HTTPResponse.hpp"
#include <sstream>

HTTPResponse::HTTPResponse()
{
	// We initialize the status code to 0 to indicate that it has not been set
	_statusCode = 0;
	_isCGI = false;
	// We should initialize the pipe file descriptors to -1 to indicate that they are not set
	// 0 is a valid file descriptor, so we can't use it to indicate that the pipe is not set
	_CGIpipeFD[0] = -1;
	_CGIpipeFD[1] = -1;
}

HTTPResponse::HTTPResponse(const HTTPResponse &other)
	: _statusCode(other._statusCode), _headers(other._headers), _body(other._body), _isCGI(other._isCGI)
{
	_CGIpipeFD[0] = other._CGIpipeFD[0];
	_CGIpipeFD[1] = other._CGIpipeFD[1];
}
void HTTPResponse::setErrorResponse(int statusCode)
{
	std::string statusMessage = getStatusMessage(statusCode);
	std::string code = toString(statusCode);
	Debug::log("statusCode: " + code + " statusMessage: " + statusMessage, Debug::NORMAL);
	std::string body = "<html><head><title>Error</title></head>"
					   "<body><h1>Error: " +
					   code + " " + "</h1><p>" + statusMessage + "</p></body></html>";

	Debug::log("setErrorResponse: statusCode: " + code + " statusMessage: " + statusMessage +
				   " body: " + body,
			   Debug::NORMAL);
	setStatusCode(statusCode, "");
	setHeader("Content-Length", toString(body.length()));
	setHeader("Content-Type", "text/html");
	setBody(body);
}

std::string HTTPResponse::objToString() const
{
	std::stringstream responseStream;
	if (_statusCode == 0)
	{
		Debug::log("Sending a response with status code 0", Debug::NORMAL);
	}
	responseStream << "HTTP/1.1 " << _statusCode << " " << getStatusMessage(_statusCode) << "\r\n";
	for (size_t i = 0; i < _headers.size(); ++i)
	{
		responseStream << _headers[i].first << ": " << _headers[i].second << "\r\n";
	}

	responseStream << "\r\n";
	responseStream << _body;
	return responseStream.str();
}

HTTPResponse &HTTPResponse::operator=(const HTTPResponse &other)
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_headers = other._headers;
		_body = other._body;
		_isCGI = other._isCGI;
		_CGIpipeFD[0] = other._CGIpipeFD[0];
		_CGIpipeFD[1] = other._CGIpipeFD[1];
	}
	return *this;
}

HTTPResponse::~HTTPResponse()
{
}

int HTTPResponse::getStatusCode() const
{
	return _statusCode;
}

void HTTPResponse::setStatusCode(int statusCode, const std::string &message)
{
	if (!message.empty())
		Debug::log(message, Debug::NORMAL);
	if (_statusCode != 0)
	{
		Debug::log("Warning: Overwriting existing status code (" + toString(_statusCode) + ") and message (" +
					   _statusMessage + ") with new code (" + toString(statusCode) + ") and message (" +
					   getStatusMessage(statusCode) + ").",
				   Debug::NORMAL);
	}

	_statusCode = statusCode;
	_statusMessage = getStatusMessage(statusCode);
}

void HTTPResponse::setHeader(const std::string &name, const std::string &value)
{
	_headers.push_back(std::make_pair(name, value));
}

void HTTPResponse::setBody(const std::string &body)
{
	_body = body;
	std::ostringstream oss;
	// oss << body.size();
	// setHeader("Content-Length", oss.str());
}

std::string HTTPResponse::getBody() const
{
	return _body;
}

std::string HTTPResponse::getHeader(const std::string &name) const
{
	for (size_t i = 0; i < _headers.size(); ++i)
	{
		if (_headers[i].first == name)
		{
			return _headers[i].second;
		}
	}
	return "";
}

bool HTTPResponse::getIsCGI() const
{
	return _isCGI;
}

void HTTPResponse::setIsCGI(bool value)
{
	_isCGI = value;
}

int (&HTTPResponse::getCGIpipeFD())[2]
{
	return _CGIpipeFD;
}

void HTTPResponse::setCGIpipeFD(int (&pipe)[2])
{
	_CGIpipeFD[0] = pipe[0];
	_CGIpipeFD[1] = pipe[1];
}

void HTTPResponse::CGIStringToResponse(const std::string &cgiOutput)
{
	// std::cout << YELLOW << cgiOutput << RESET << std::endl;
	std::size_t headerEndPos = cgiOutput.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = cgiOutput.find("\n\n");
	}

	std::string headersPart = cgiOutput.substr(0, headerEndPos);
	// std::cout << "Headers: " << headersPart << std::endl;
	std::string bodyPart = cgiOutput.substr(headerEndPos);

	Debug::log("------------------CGIStringToResponse-------------------", Debug::CGI);

	std::istringstream headerStream(headersPart);
	std::string headerLine;
	while (std::getline(headerStream, headerLine))
	{
		if (!headerLine.empty() && headerLine[headerLine.size() - 1] == '\r')
		{
			headerLine.erase(headerLine.size() - 1); // carriage return
		}

		std::size_t separatorPos = headerLine.find(": ");
		if (separatorPos != std::string::npos)
		{
			std::string headerName = headerLine.substr(0, separatorPos);
			std::string headerValue = headerLine.substr(separatorPos + 2);
			setHeader(headerName, headerValue);
			// std::cout << "Header: " << headerName << ": " << headerValue << std::endl;
			if (headerName == "Status")
			{
				std::size_t spacePos = headerValue.find(" ");
				if (spacePos != std::string::npos)
				{
					std::string statusCodeStr = headerValue.substr(0, spacePos);
					int statusCode = std::atoi(statusCodeStr.c_str());
					setStatusCode(statusCode, "");
				}
			}
		}
	}

	setBody(bodyPart);
	// At his point we are done with the CGI so setIsCGI(false)
	// setIsCGI(true);
	if (_statusCode == 0)
		setStatusCode(200, "OK");
	return;
}

std::string HTTPResponse::getStatusMessage(int statusCode) const
{
	switch (statusCode)
	{
	case 100:
		return "Continue";
	case 101:
		return "Switching Protocols";
	case 102:
		return "Processing"; // WebDAV; RFC 2518
	case 103:
		return "Early Hints";
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 203:
		return "Non-Authoritative Information";
	case 204:
		return "No Content";
	case 205:
		return "Reset Content";
	case 206:
		return "Partial Content";
	case 207:
		return "Multi-Status"; // WebDAV; RFC 4918
	case 208:
		return "Already Reported"; // WebDAV; RFC 5842
	case 226:
		return "IM Used"; // HTTP Delta encoding; RFC 3229
	case 300:
		return "Multiple Choices";
	case 301:
		return "Moved Permanently";
	case 302:
		return "Found";
	case 303:
		return "See Other";
	case 304:
		return "Not Modified";
	case 305:
		return "Use Proxy";
	case 307:
		return "Temporary Redirect";
	case 308:
		return "Permanent Redirect"; // RFC 7538
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 402:
		return "Payment Required";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 406:
		return "Not Acceptable";
	case 407:
		return "Proxy Authentication Required";
	case 408:
		return "Request Timeout";
	case 409:
		return "Conflict";
	case 410:
		return "Gone";
	case 411:
		return "Length Required"; // ✅in Parser.cpp
	case 412:
		return "Precondition Failed";
	case 413:
		return "Payload Too Large";
	case 414:
		return "URI Too Long"; // not present in NGINX
	case 415:
		return "Unsupported Media Type"; // ✅in Parser.cpp
	case 416:
		return "Range Not Satisfiable";
	case 417:
		return "Expectation Failed";
	case 418:
		return "I'm a teapot"; // RFC 2324, RFC 7168
	case 421:
		return "Misdirected Request"; // RFC 7540
	case 422:
		return "Unprocessable Entity"; // WebDAV; RFC 4918
	case 423:
		return "Locked"; // WebDAV; RFC 4918
	case 424:
		return "Failed Dependency"; // WebDAV; RFC 4918
	case 425:
		return "Too Early"; // RFC 8470
	case 426:
		return "Upgrade Required";
	case 428:
		return "Precondition Required"; // RFC 6585
	case 429:
		return "Too Many Requests"; // RFC 6585
	case 431:
		return "Request Header Fields Too Large"; // ✅in Server.cpp, RFC 6585
	case 451:
		return "Unavailable For Legal Reasons"; // RFC 7725
	case 499:
		return "Client Closed Request"; // nginx
	case 500:
		return "Internal Server Error";
	case 501:
		return "Not Implemented";
	case 502:
		return "Bad Gateway";
	case 503:
		return "Service Unavailable";
	case 504:
		return "Gateway Timeout";
	case 505:
		return "HTTP Version Not Supported"; // ✅in Parser.cpp
	case 506:
		return "Variant Also Negotiates"; // RFC 2295
	case 507:
		return "Insufficient Storage"; // WebDAV; RFC 4918
	case 508:
		return "Loop Detected"; // WebDAV; RFC 5842
	case 510:
		return "Not Extended"; // RFC 2774
	case 511:
		return "Network Authentication Required"; // RFC 6585
	default:
	{
		std::cerr << RED << "Unassigned status code: " << statusCode << RESET << std::endl;
		return "Unassigned Status Code";
	}
	}
}

const std::string &HTTPResponse::getStatusMessage() const
{
	return _statusMessage;
}

std::ostream &operator<<(std::ostream &out, const HTTPResponse &response)
{
	// Output the status
	out << "\033[35m";
	out << "Status Code: " << response.getStatusCode() << "\n";
	// out << "Status Message: " << response.getStatusMessage() << "\n";

	// Output headers
	out << "Headers:\n";
	for (size_t i = 0; i < response._headers.size(); ++i)
	{
		out << response._headers[i].first << ": " << response._headers[i].second << "\n";
	}

	// Output body
	if (!response.getBody().empty())
	{
		out << "\nBody:\n" << response.getBody();
	}
	out << "\033[0m";
	return out;
}
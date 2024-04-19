#include "Parser.hpp"
#include <iomanip>

Parser::Parser()
{
	// Fill me
}

Parser::~Parser()
{
}

// Parser::Parser(const char *request)
// {
// 	_statusCode = 200;
// 	if (strlen(request) < 10)
// 		ft_error(400, "Invalid request-line");
// 	else
// 	{
// 		parseRequestLine(request);
// 		if (_statusCode == 200)
// 			parseHeaders(request);
// 		if (_statusCode == 200 && !_isChunked && _method != "GET")
// 			parseBody(request);
// 	}
// }

bool Parser::getIsChunked() const
{
	return (_isChunked);
}

bool Parser::getIsChunkFinish() const
{
	return (_isChunkFinish);
}

// std::string HTTPRequest::getErrorMessage() const
// {
// 	return (_errorMessage);
// }

void Parser::setIsChunked(bool n)
{
	_isChunked = n;
}

// int HTTPRequest::ft_error(int statusCode, std::string message)
// {
// 	_errorMessage = message;
// 	_statusCode = statusCode;
// 	return (statusCode);
// }

void printHTTPRequest(const std::string httpRequest, size_t startPos)
{
	std::cout << "HTTP Request with not printables:" << std::endl;
	for (size_t i = startPos; i < httpRequest.length(); ++i)
	{
		unsigned char c = httpRequest[i];
		{
			switch (c)
			{
			case '\n':
				std::cout << "\\n";
				break;
			case '\r':
				std::cout << "\\r";
				break;
			case '\t':
				std::cout << "\\t";
				break;
			default:
				if (c >= 32 && c <= 126)
					std::cout << c;
				else
					std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
			}
		}
	}
	std::cout << std::endl;
}

void Parser::parseRequestLine(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int i = 0;
	bool isOriginForm = false;
	printHTTPRequest(request);
	// We will work with a Response object with a status code of 0. If we encounter an error, we will set the status
	// code to the appropriate value. The code 200 was checked in the constructor after the call of parseRequestLine. If
	// the status code is not 200, we would have returned otherwise we would have proceed parsing the headers and after
	// another similar check parsing the body. _statusCode = 200;

	// TODO: check if strlen is the best C++ option
	if (strlen(request) < 10)
	{
		std::cerr << "Invalid request-line" << std::endl;
		return (res.setStatusCode(400));
	}

	std::string method = extractMethod(request, i);
	if (method.empty())
	{
		// TODO: 501 or 400?
		return (res.setStatusCode(501));
	}
	if (request[i++] != ' ')
	{
		std::cerr << "Invalid request-line" << std::endl;
		return (res.setStatusCode(400));
	}
	req.setMethod(method);

	std::string requestTarget = extractRequestTarget(request, i);
	std::cout << std::endl << std::endl << "request                            Target = " << requestTarget << std::endl;
	if (requestTarget.empty())
	{
		std::cerr << "Invalid request-target" << std::endl;
		return (res.setStatusCode(414));
	}
	if (request[i++] != ' ')
	{
		std::cerr << "Invalid request-line" << std::endl;
		return (res.setStatusCode(400));
	}
	req.setRequestTarget(requestTarget);

	// TODO: which variables are we talking about?
	std::string variables = extractVariables(requestTarget, isOriginForm);
	if (variables.empty())
		return (res.setStatusCode(400));
	if (isOriginForm && !saveVariables(variables, req))
		return (res.setStatusCode(400));

	std::string protocolVersion = extractProtocolVersion(request, i);
	if (protocolVersion.empty())
		return (res.setStatusCode(400));
	if (!hasCRLF(request, i, 0))
		return (res.setStatusCode(400));
	req.setProtocolVersion(protocolVersion);
	std::cout << "before res.getStatusCode() = " << res.getStatusCode() << std::endl;

	if (res.getStatusCode() == 0)
		parseHeaders(request, req, res);
	std::cout << "after res.getStatusCode() = " << res.getStatusCode() << std::endl;

	if (res.getStatusCode() == 0)
		parseBody(request, req, res);
}
// TODO: probably we will remove this. The parser will always get a chunk of a chunked body and not the whole chunked
// body, cause the 'core' will read only a chunk of the body at a time
void Parser::parseChunkedBody(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int i = 0;
	int size = 0;
	std::string line;

	skipHeader(request, i);
	while (request[i])
	{
		size = extractLineLength(request, i);
		if (size <= 0)
		{
			if (size == -1)
			{
				std::cerr << "Invalid chunk size" << std::endl;
				return (res.setStatusCode(400));
			}
			break;
		}
		line = extractLine(request, i, size);
		if (line.empty())
		{
			std::cerr << "Invalid chunk" << std::endl;
			return (res.setStatusCode(400));
		}
		req.setBody(line);
	}
	if (!hasCRLF(request, i, 0))
	{
		std::cerr << "No CRLF after chunked body" << std::endl;
		return (res.setStatusCode(400));
	}
	if (hasCRLF(request, i, 1))
		_isChunkFinish = true;
}

void Parser::parseHeaders(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int i;
	std::string key;
	std::string value;
	printHTTPRequest(request);
	i = 0;
	// TODO: check if we need to skip the request line in the new implementation
	skipRequestLine(request, i);
	while (request[i])
	{
		key = extractHeaderKey(request, i);
		if (key.empty())
		{
			std::cerr << "Invalid header key" << std::endl;
			return (res.setStatusCode(400));
		}
		i++; // skip ':'
		if (request[i++] != ' ')
		{
			std::cerr << "Invalid header value" << std::endl;
			return (res.setStatusCode(400));
		}
		value = extractHeaderValue(request, i);
		if (value.empty())
		{
			std::cerr << "Invalid header value" << std::endl;
			return (res.setStatusCode(400));
		}
		std::cout << "res.getStatusCode() = " << res.getStatusCode() << std::endl;
		if (!hasCRLF(request, i, 0))
		{
			std::cerr << "No CRLF after header" << std::endl;
			return (res.setStatusCode(400));
		}
		req.setHeaders(key, value);
		i += 2;						// skip '\r' and '\n'
		if (hasCRLF(request, i, 0)) // end of header section
			break;
	}
	if (!hasCRLF(request, i, 0))
	{
		std::cerr << "No CRLF after headers" << std::endl;
		return (res.setStatusCode(400));
	}
	// TODO: can't we do them lower case in the setHeaders method?
	// setHeaders is now dirctly lowercasing the key
	// makeHeadersLowCase();
	if (!hasMandatoryHeaders(req))
	{
		std::cerr << "Invalid headers" << std::endl;
		return (res.setStatusCode(400));
	}
	// TODO: body in a GET request is not explicitly forbidden
	if (req.getMethod() == "GET" && request[i + 2]) // has something after headers
	{
		std::cerr << "GET request with body" << std::endl;
		return (res.setStatusCode(400));
	}
}

// TODO: IMO this is pretty confusing
void Parser::parseBody(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int end = 400;
	unsigned int i = 0;
	unsigned int start = 0;
	std::string stringRequest(request);
	std::vector<std::string> body;

	skipHeader(request, i);
	start = i;
	while (request[i] && end != 200)
	{
		if (hasCRLF(request, i, 0))
		{
			if (hasCRLF(request, i, 1))
				end = 200;
			// body.push_back(stringRequest.substr(start, i - start));
			req.setBody(stringRequest.substr(start, i - start));
			i += 2;
			start = i;
			continue;
		}
		else if (!hasCRLF(request, i, 0) && request[i] == '\r')
		{
			std::cerr << "Invalid body" << std::endl;
			return (res.setStatusCode(400));
		}
		i++;
	}
	if (end == 200 && hasCRLF(request, i, 0) && request[i + 2])
	{
		std::cerr << "Invalid body" << std::endl;
		return (res.setStatusCode(400));
	}
}

// ----------------UTILS----------------------------

bool Parser::hasMandatoryHeaders(HTTPRequest &req)
{
	// int isHost = 0;
	// int isContentLength = 0;
	// int isContentType = 0;
	bool isHost = false;
	bool isContentLength = false;
	bool isContentType = false;

	std::multimap<std::string, std::string> headers = req.getHeaders();
	std::multimap<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == "host")
		{
			if (!isValidHost(it->second))
				return (false);
			isHost = true;
			// isHost++;
		}
		else if (it->first == "content-length")
		{
			if (!isNumber(it->second) || req.getMethod() != "POST")
				return (false);
			isContentLength = true;
			// isContentLength++;
		}
		else if (it->first == "content-type")
		{
			if (!isValidContentType(it->second) || req.getMethod() != "POST")
				return (false);
			isContentType = true;
			// isContentType++;
		}
		else if (it->first == "transfer-encoding")
		{
			if (it->second != "chunked" || req.getMethod() != "POST")
				return (false);
			setIsChunked(true);
		}
	}
	if (_isChunked && isContentLength)
		return (false);
	if (req.getMethod() == "POST" || req.getMethod() == "DELETE")
		return (isHost && isContentLength && isContentType);
	// return (isHost == 1 && isContentLength == 1 && isContentType == 1);
	else
		return (isHost);
	// return (isHost == 1);
}

bool Parser::saveVariables(std::string &variables, HTTPRequest &req)
{
	int startPos = 0;
	std::string key;
	std::string value;

	for (int i = 0; i < (int)variables.length(); i++)
	{
		if (variables[i] == '=')
		{
			key = extractKey(variables, i, startPos);
			if (key.empty())
				return (false);
			value = extractValue(variables, i);
			if (value.empty())
				return (false);
			startPos = i;
			req.setQueryString(key, value);
		}
	}
	return (true);
}

// void Parser::makeHeadersLowCase()
// {
// 	std::multimap<std::string, std::string>::iterator it;
// 	std::multimap<std::string, std::string> newHeaders;
// 	std::string tmp;

// 	for (it = _headers.begin(); it != _headers.end(); ++it)
// 	{
// 		tmp = it->first;
// 		for (unsigned int i = 0; i < tmp.size(); ++i)
// 		{
// 			if (tmp[i] >= 65 && tmp[i] <= 90)
// 				tmp[i] = tmp[i] + 32;
// 		}
// 		newHeaders.insert(std::make_pair(tmp, it->second));
// 	}
// 	_headers.swap(newHeaders);
// }

std::string Parser::extractValue(std::string &variables, int &i)
{
	int startPos;

	startPos = i++;
	while (variables[i] && variables[i] != '&')
	{
		if (variables[i] == '=')
			return ("");
		i++;
	}
	if (variables[startPos] == '=')
		startPos++;
	if (variables[i] == '&' && !variables[i + 1])
		return ("");
	return (variables.substr(startPos, i - startPos));
}

std::string Parser::extractKey(std::string &variables, int &i, int startPos)
{
	if (i == 0 || variables[i] == '?')
		return ("");
	if (variables[startPos] == '&' && startPos != 0)
		startPos++;
	if (variables.substr(startPos, i - startPos).find('&') != std::string::npos ||
		variables.substr(startPos, i - startPos).find('?') != std::string::npos)
		return ("");
	return (variables.substr(startPos, i - startPos));
}

std::string Parser::extractRequestTarget(const char *request, unsigned int &i)
{
	std::string requestTarget;
	std::string string_request(request);
	unsigned int start = i;

	while (request[i] && request[i] != ' ' && !isInvalidChar(request[i]))
		i++;
	if (i > MAX_URI)
		return ("");
	requestTarget = string_request.substr(start, i - start);
	return (requestTarget);
}

std::string Parser::extractVariables(std::string &requestTarget, bool &isOriginForm)
{
	int queryStart = 0;

	if (requestTarget == "/")
		return ("/");
	isOriginForm = isOrigForm(requestTarget, queryStart);
	if (isOriginForm)
		// return (requestTarget.substr(queryStart + 1); // better version
		return (requestTarget.substr(queryStart + 1, strlen(requestTarget.c_str()) - queryStart));
	return (requestTarget);
}

std::string Parser::extractProtocolVersion(const char *request, unsigned int &i)
{
	std::string protocolVersion;
	std::string string_request(request);
	unsigned int start = i;

	while (request[i] && request[i] != '\r' && !isInvalidChar(request[i]))
		i++;
	protocolVersion = string_request.substr(start, i - start);
	if (protocolVersion == "HTTP/1.1")
		return (protocolVersion);
	return ("");
}

std::string Parser::extractMethod(const char *request, unsigned int &i)
{
	std::string method;
	std::string string_request(request);

	while (request[i] && request[i] != ' ' && !isInvalidChar(request[i]))
		i++;
	method = string_request.substr(0, i);
	if (method == "GET" || method == "POST" || method == "DELETE")
		return (method);
	return ("");
}

std::string Parser::extractHeaderKey(const char *request, unsigned int &i)
{
	std::string string_request(request);
	unsigned int start = i;

	while (request[i] && request[i] != ':')
	{
		if (isInvalidChar(request[i]) || request[i] == ' ')
			return ("");
		i++;
	}
	return (string_request.substr(start, i - start));
}

std::string Parser::extractHeaderValue(const char *request, unsigned int &i)
{
	std::string string_request(request);
	unsigned int start = i;

	while (request[i] && request[i] != '\r')
	{
		if (isInvalidChar(request[i]))
			return ("");
		i++;
	}
	return (string_request.substr(start, i - start));
}

unsigned int Parser::extractLineLength(const char *request, unsigned int &i)
{
	std::string string_request(request);
	unsigned int start = i;
	int size = 0;

	while (request[i] && request[i] != '\r')
		i++;
	if (request[i] != '\r' || request[i + 1] != '\n')
		return (-1);
	size = hexToInt(string_request.substr(start, i - start));
	if (size <= 0)
		return (size);
	i += 2; // skip '\r' and '\n'
	return (size);
}

std::string Parser::extractLine(const char *request, unsigned int &i, const unsigned int &size)
{
	std::string string_request(request);
	std::string line = string_request.substr(i, size);
	i += size; // skip line
	if (request[i] != '\r' || request[i + 1] != '\n')
		return ("");
	i += 2; // skip '\r' and '\n'
	return (line);
}

bool Parser::isOrigForm(std::string &requestTarget, int &queryStart)
{
	for (int i = 0; i < (int)requestTarget.length(); i++)
	{
		if (requestTarget[i] == '?')
		{
			queryStart = i;
			return (true);
		}
	}
	return (false);
}

bool Parser::isValidContentType(std::string type)
{
	if (type == "text/plain" || type == "text/html")
		return (true);
	return (false);
}

bool Parser::isValidHost(std::string host)
{
	(void)host;
	return (true);
}

void Parser::skipRequestLine(const char *request, unsigned int &i)
{
	while (request[i])
	{
		if (hasCRLF(request, i, 0))
		{
			i += 2;
			return;
		}
		i++;
	}
}

void Parser::skipHeader(const char *request, unsigned int &i)
{
	while (request[i])
	{
		if (hasCRLF(request, i, 1))
		{
			i += 4; // skip "\r\n\r\n"
			return;
		}
		i++;
	}
}
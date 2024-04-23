#include "Parser.hpp"
#include "HTTPRequest.hpp"
#include "Server.hpp"
#include "server_utils.hpp"

Parser::Parser()
{
	// Fill me
	_isChunked = false;
	_isChunkFinish = false;
	_headersComplete = false;
	_bodyComplete = false;
	_buffer = "";
	_clientMaxHeadersSize = CLIENT_MAX_HEADERS_SIZE;
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

// FUNCTIONS FROM THE CORE PARSING FUNCTIONALITIES

bool Parser::preParseHeaders(HTTPResponse &res)
{
	std::cout << "\nEntering preParseHeaders" << std::endl;
	// We read the buffer with readSocket if headersComplete is not true and we write the buffer in the _headersBuffer
	std::size_t headersEnd = _buffer.find("\r\n\r\n");
	if (headersEnd != std::string::npos)
	{
		// std::string headers;
		// headers = _buffer.substr(0, headersEnd);
		// setHeadersBuffer(headers);
		// setHeadersComplete(true);
		_headersBuffer = _buffer.substr(0, headersEnd + 4);
		_headersComplete = true;
		_buffer = _buffer.substr(headersEnd + 4);
		_bodyTotalBytesRead = _buffer.length();
		std::cout << "Exiting preParseHeaders: true #1" << std::endl;
		return (true);
	}
	_headersTotalBytesRead = _buffer.length();
	if (_headersTotalBytesRead > _clientMaxHeadersSize)
	{
		std::cerr << "Headers too large" << std::endl;
		res.setStatusCode(431);
		return false;
	}
	std::cout << "Exiting preParseHeaders: true #2" << std::endl;
	return true;
}

// GETTERS FROM THE CORE PARSING FUNCTIONALITIES
bool Parser::getHeadersComplete() const
{
	return (_headersComplete);
}

std::string Parser::getBuffer() const
{
	return (_buffer);
}

std::string Parser::getHeadersBuffer() const
{
	return (_headersBuffer);
}

size_t Parser::getBodyTotalBytesRead() const
{
	return (_bodyTotalBytesRead);
}

size_t Parser::getHeadersTotalBytesRead() const
{
	return (_headersTotalBytesRead);
}

bool Parser::getBodyComplete() const
{
	return (_bodyComplete);
}

// SETTERS FROM THE CORE PARSING FUNCTIONALITIES
void Parser::setHeadersComplete(bool value)
{
	_headersComplete = value;
}

void Parser::setBuffer(std::string str)
{
	_buffer = str;
}

void Parser::setHeadersBuffer(std::string str)
{
	_headersBuffer = str;
}

void Parser::setBodyTotalBytesRead(size_t value)
{
	_bodyTotalBytesRead = value;
}

void Parser::setHeadersTotalBytesRead(size_t value)
{
	_headersTotalBytesRead = value;
}

void Parser::setBodyComplete(bool value)
{
	_bodyComplete = value;
}

// VARIABLES

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

void Parser::setIsChunked(bool value)
{
	_isChunked = value;
}

// int HTTPRequest::ft_error(int statusCode, std::string message)
// {
// 	_errorMessage = message;
// 	_statusCode = statusCode;
// 	return (statusCode);
// }

void Parser::parseRequestLine(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	std::cout << "\nEntering parseRequestLine" << std::endl;

	unsigned int i = 0;
	bool isOriginForm = false;
	printStrWithNonPrintables(request, 0);

	// TODO: check if strlen is the best C++ option

	std::string method = extractMethod(request, i);
	if (method.empty())
	{
		std::cerr << "Invalid request-line" << std::endl;
		std::cerr << "method: " << method << std::endl;
		// TODO: 501 or 400?
		return (res.setStatusCode(501));
	}
	if (request[i++] != ' ')
	{
		std::cerr << "Invalid request-line" << std::endl;
		return (res.setStatusCode(400));
	}
	std::cout << "method: " << method << std::endl;
	req.setMethod(method);

	std::string requestTarget = extractRequestTarget(request, i);
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
	std::cout << "Exiting parseRequestLine" << std::endl;
}

void Parser::parseRequest(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	std::cout << "\nEntering parseRequest" << std::endl;
	// We will work with a Response object with a status code of 0. If we encounter an error, we will set the status
	// code to the appropriate value. The code 200 was checked in the constructor after the call of parseRequest. If
	// the status code is not 200, we would have returned otherwise we would have proceed parsing the headers and after
	// another similar check parsing the body. _statusCode = 200;
	if (strlen(request) < 10)
	{
		std::cerr << "Invalid request-line" << std::endl;
		return (res.setStatusCode(400));
	}
	else
	{
		parseRequestLine(request, req, res);
		if (res.getStatusCode() == 0)
			parseHeaders(request, req, res);
		std::cout << "after res.getStatusCode() = " << res.getStatusCode() << std::endl;
		if (res.getStatusCode() == 0 && !_isChunked && req.getMethod() != "GET")
			parseBody(request, req, res);
	}
	std::cout << "Exiting parseRequest" << std::endl;
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
	std::cout << "\nEntering parseHeaders" << std::endl;
	unsigned int i;
	std::string key;
	std::string value;
	printStrWithNonPrintables(request, 0);
	i = 0;
	// TODO: check if we need to skip the request line in the new implementation
	skipRequestLine(request, i);
	while (request[i])
	{
		key = extractHeaderKey(request, i);
		std::cout << "key: " << key << std::endl;
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
		std::cout << "value: " << value << std::endl;
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
	std::cout << "Exiting parseHeaders" << std::endl;
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
	std::cout << "\nEntering hasMandatoryHeaders" << std::endl;
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
			std::cout << "checking content-length" << std::endl;
			// if (!isNumber(it->second) || req.getMethod() != "POST")
			if (!isNumber(it->second))
			{
				std::cerr << "Invalid content-length" << std::endl;
				std::cerr << "it->second: " << it->second << std::endl;
				return (false);
			}
			req.setHasContentLengthHeader(true);
			// req.setContentLength(std::stoi(it->second));
			std::istringstream iss(it->second);
			int contentLength;

			iss >> contentLength;
			if (iss.fail())
			{
				std::cerr << "conversion failed" << std::endl;
				return (false);
			}
			std::cout << "contentLength: " << contentLength << std::endl;
			req.setContentLength(contentLength);
			isContentLength = true;
			// isContentLength++;
		}
		else if (it->first == "content-type")
		{
			std::cout << "checking content-type" << std::endl;
			std::cout << "method: " << req.getMethod() << std::endl;
			if (!isValidContentType(it->second) || req.getMethod() != "POST")
			{
				std::cerr << "Invalid content-type" << std::endl;
				return (false);
			}
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
	std::cout << "Entering extractMethod" << std::endl;
	std::string method;
	std::string string_request(request);

	while (request[i] && request[i] != ' ' && !isInvalidChar(request[i]))
		i++;
	method = string_request.substr(0, i);
	if (method == "GET" || method == "POST" || method == "DELETE")
	{
		std::cout << "Exiting extractMethod with method: " << method << std::endl;
		return (method);
	}
	std::cout << "Exiting extractMethod with "
				 ""
			  << std::endl;
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
	std::cout << "Entering isValidContentType" << std::endl;
	std::cout << "type: " << type << std::endl;
	if (type == "text/plain" || type == "text/html")
	{
		std::cout << "Exiting isValidContentType: true!" << std::endl;
		return (true);
	}
	std::cout << "Exiting isValidContentType: false!" << std::endl;
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
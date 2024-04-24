#include "Parser.hpp"
#include <iomanip>

Parser::Parser()
{
	// Fill me
}

Parser::~Parser()
{
}

bool Parser::getIsChunked() const
{
	return (_isChunked);
}

bool Parser::getIsChunkFinish() const
{
	return (_isChunkFinish);
}

void Parser::setIsChunked(bool n)
{
	_isChunked = n;
}

void Parser::parseRequest(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	if (strlen(request) < 10)
		return (res.setStatusCode(400, "Invalid request-line"));
	else
	{
		parseRequestLine(request, req, res);
		if (res.getStatusCode() == 0)
			parseHeaders(request, req, res);
		if (res.getStatusCode() == 0 && !_isChunked && req.getMethod() != "GET")
			parseBody(request, req, res);
	}
}

void Parser::parseRequestLine(const char *request, HTTPRequest &req, HTTPResponse &res)
{

	unsigned int i = 0;
	bool isOriginForm = false;

	std::string method = extractMethod(request, i);
	if (method.empty())
		return (res.setStatusCode(501, "Invalid method"));
	if (request[i++] != ' ')
		return (res.setStatusCode(400, "Invalid request-line"));

	std::string requestTarget = extractRequestTarget(request, i);
	if (requestTarget.empty())
		return (res.setStatusCode(414, "Invalid request-target"));
	if (request[i++] != ' ')
		return (res.setStatusCode(400, "Invalid request-line"));

	std::string variables = extractVariables(requestTarget, isOriginForm);
	if (variables.empty())
		return (res.setStatusCode(400, "Invalid variables in request-line"));
	if (isOriginForm && !saveVariables(variables, req))
		return (res.setStatusCode(400, "Invalid variables in request-line"));

	std::string protocolVersion = extractProtocolVersion(request, i);
	if (protocolVersion.empty())
		return (res.setStatusCode(400, "Invalid protocol version"));
	if (!hasCRLF(request, i, 0))
		return (res.setStatusCode(400, "No CRLF at the end of request-line"));

	req.setMethod(method);
	req.setRequestTarget(requestTarget);
	req.setProtocolVersion(protocolVersion);
}

// [KEY][:][SP][VALUE][CRLF][KEY][:][SP][VALUE][CRLF][CRLF]
void Parser::parseHeaders(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int i = 0;
	std::string key;
	std::string value;

	skipRequestLine(request, i);
	while (request[i])
	{
		key = extractHeaderKey(request, i); // [KEY]
		if (key.empty())
			return (res.setStatusCode(400, "Invalid header key"));
		if (request[i++] != ':') // [:]
			return (res.setStatusCode(400, "Invalid header key"));
		if (request[i++] != ' ') // [SP]
			return (res.setStatusCode(400, "Invalid header key"));

		value = extractHeaderValue(request, i); // [VALUE]
		if (value.empty())
			return (res.setStatusCode(400, "Invalid header value"));
		if (!hasCRLF(request, i, 0)) // [CRLF]
			return (res.setStatusCode(400, "No CRLF after header"));
		i += 2; // skip [CRLF]

		req.setHeaders(key, value);
		if (hasCRLF(request, i, 0)) // end of header section ([CRLF][CRLF])
			break;
	}
	if (!hasCRLF(request, i, 0))
		return (res.setStatusCode(400, "No CRLF after headers"));
	if (!hasMandatoryHeaders(req))
		return (res.setStatusCode(400, "Invalid headers"));
}

// [LINE][CRLF][LINE][CRLF][CRLF]
void Parser::parseBody(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	unsigned int i = 0;
	unsigned int start = 0;
	std::string stringRequest(request);
	std::vector<std::string> body;

	skipHeader(request, i);
	start = i;
	while (request[i])
	{
		if (hasCRLF(request, i, 0)) // [CRLF]
		{
			if (hasCRLF(request, i, 1)){ // [CRLF][CRLF]
				req.setBody(stringRequest.substr(start, i - start)); // [LINE]
				break ;
			}
			req.setBody(stringRequest.substr(start, i - start)); // [LINE]
			i += 2; // skip [CRLF]
			start = i;
			continue;
		}
		i++;
	}
	if (!hasCRLF(request, i, 1) || request[i + 4]) // if no [CRLF][CRLF] or there is something after body 
		return (res.setStatusCode(400, "Invalid body"));
}

// // TODO: probably we will remove this. The parser will always get a chunk of a chunked body and not the whole chunked
// // body, cause the 'core' will read only a chunk of the body at a time
// void Parser::parseChunkedBody(const char *request, HTTPRequest &req, HTTPResponse &res)
// {
// 	unsigned int i = 0;
// 	int size = 0;
// 	std::string line;

// 	skipHeader(request, i);
// 	while (request[i])
// 	{
// 		size = extractLineLength(request, i);
// 		if (size <= 0)
// 		{
// 			if (size == -1)
// 			{
// 				std::cerr << "Invalid chunk size" << std::endl;
// 				return (res.setStatusCode(400));
// 			}
// 			break;
// 		}
// 		line = extractLine(request, i, size);
// 		if (line.empty())
// 		{
// 			std::cerr << "Invalid chunk" << std::endl;
// 			return (res.setStatusCode(400));
// 		}
// 		req.setBody(line);
// 	}
// 	if (!hasCRLF(request, i, 0))
// 	{
// 		std::cerr << "No CRLF after chunked body" << std::endl;
// 		return (res.setStatusCode(400));
// 	}
// 	if (hasCRLF(request, i, 1))
// 		_isChunkFinish = true;
// }

// ----------------UTILS----------------------------

bool Parser::hasMandatoryHeaders(HTTPRequest &req)
{
	int isHost = 0;
	int isContentLength = 0;
	int isContentType = 0;

	std::multimap<std::string, std::string> headers = req.getHeaders();
	std::multimap<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == "host")
		{
			if (!isValidHost(it->second))
				return (false);
			isHost++;
		}
		else if (it->first == "content-length")
		{
			if (!isNumber(it->second))
				return (false);
			isContentLength++;
		}
		else if (it->first == "content-type")
		{
			if (!isValidContentType(it->second))
				return (false);
			isContentType++;
		}
		else if (it->first == "transfer-encoding")
		{
			if (it->second != "chunked")
				return (false);
			setIsChunked(true);
		}
	}
	// if (_isChunked && isContentLength)
	// 	return (false);
	if (req.getMethod() == "POST" || req.getMethod() == "DELETE")
		return (isHost == 1 && isContentLength == 1 && isContentType == 1);
	else
		return (isHost == 1);
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
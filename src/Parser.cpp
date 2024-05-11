#include "Parser.hpp"
#include "HTTPRequest.hpp"
#include "Server.hpp"
#include "server_utils.hpp"

int strToInt(const std::string &str);

Parser::Parser()
{
	_isChunked = false;
	_headersComplete = false;
	_bodyComplete = false;
	_headersAreParsed = false;
	_buffer = "";
	_headersBuffer = "";
}

Parser::~Parser()
{
}

bool Parser::preParseHeaders(HTTPResponse &res)
{
	// We read the buffer with readHeaders if headersComplete is not true and we write the buffer in the _headersBuffer
	std::size_t headersEnd = _buffer.find("\r\n\r\n");
	if (headersEnd != std::string::npos)
	{
		_headersBuffer = _buffer.substr(0, headersEnd + 4);
		_headersComplete = true;
		_buffer = _buffer.substr(headersEnd + 4);
		std::cout << _buffer << std::endl;
		return (true);
	}
	std::cout << "headers are not complete" << std::endl;
	if (_buffer.length() > CLIENT_MAX_HEADERS_SIZE)
		return (res.setStatusCode(431, "Headers too large"), false);
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

void Parser::setBodyComplete(bool value)
{
	_bodyComplete = value;
}

// VARIABLES

bool Parser::getIsChunked() const
{
	return (_isChunked);
}

void Parser::setIsChunked(bool value)
{
	_isChunked = value;
}

bool Parser::getHeadersAreParsed() const
{
	return (_headersAreParsed);
}

void Parser::parseRequestLineAndHeaders(const char *request, HTTPRequest &req, HTTPResponse &res)
{
	if (strlen(request) < 10)
		return (res.setStatusCode(400, "Invalid request-line"));
	else
	{
		parseRequestLine(request, req, res);
		if (res.getStatusCode() == 0)
			parseHeaders(request, req, res);
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
	if (!hasMandatoryHeaders(req, res))
		return ;
	_headersAreParsed = true;
	saveCokies(req);
}

// [--BOUNDARY][CRLF][HEADERS][CRLF][DATA][CRLF][--BOUNDARY--]
void Parser::parseFileUpload(const std::string &request, HTTPRequest &req, HTTPResponse &res)
{
	bool hasFilestoRead = true;
	unsigned int i = 0;
	unsigned int start = 0;
	std::string data;
	size_t boundaryIndex = 0;
	std::string upBound = req.getUploadBoundary();

	// Check the very first upload boundary
	if (!isUploadBoundary(request, req, i))
		return (res.setStatusCode(400, "Incorrect first upload boubdary"));

	// read ALL data
	start = i;
	while (i < request.length())
		i++;
	data = request.substr(start, i - start);
	while (hasFilestoRead)
	{
		// get the uploadBoundary
		boundaryIndex = data.find("--" + upBound + "\r\n");
		// if final upload boundary == end of request
		if (boundaryIndex == std::string::npos)
		{
			boundaryIndex = data.find("--" + upBound + "--");
			hasFilestoRead = false;
		}
		if (boundaryIndex == std::string::npos)
			return (res.setStatusCode(400, "Incorrect file boundary"));
		// get the file (headers + body + boundary + CRLF) and save
		if(!saveFile(data.substr(0, boundaryIndex - 2), req))
			return (res.setStatusCode(400, "Incorrect file data"));
		// erase saved data
		data.erase(0, boundaryIndex + ("----" + upBound).size());
	}
}

// ----------------UTILS----------------------------

bool Parser::hasMandatoryHeaders(HTTPRequest &req, HTTPResponse& res)
{
	_isChunked = false;
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
				return (res.setStatusCode(400, "Invalid host"), false);
			isHost++;
		}
		else if (it->first == "content-length")
		{
			if (!isNumber(it->second))
				return (res.setStatusCode(400, "Invalid content-length"), false);
			isContentLength++;
		}
		else if (it->first == "content-type")
		{
			if (!isValidContentType(it->second))
				return (res.setStatusCode(400, "Not supported content-type"), false);
			if (it->second.substr(0, 30) == "multipart/form-data; boundary=")
				req.setUploadBoundary(extractUploadBoundary(it->second));
			isContentType++;
		}
		else if (it->first == "transfer-encoding")
		{
			if (it->second != "chunked")
				return (res.setStatusCode(400, "Not supported transfer-encoding"), false);
			_isChunked = true;
		}
	}
	if (_isChunked && req.getMethod() == "POST")
		return (isHost == 1 && isContentType == 1);
	if (req.getMethod() == "POST" || req.getMethod() == "DELETE")
	{
		if (isContentLength == 0)
			return (res.setStatusCode(411, "POST request: Length Required"), false);
		if (!(isHost == 1 && isContentLength == 1 && isContentType == 1))
			return (res.setStatusCode(400, "POST request: not enough headers to process the request"), false);
	}
	else
	{
		if (!(isHost == 1))
			return (res.setStatusCode(400, "GET request: no host header"), false);
	}
	return (true);
}

void Parser::saveCokies(HTTPRequest &req)
{
	std::multimap<std::string, std::string> headers = req.getHeaders();
	std::multimap<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == "cookie")
		{
			std::string cookie = it->second;
			std::string key;
			std::string value;
			unsigned int start = 0;

			for (unsigned int i = 0; i < cookie.length(); i++)
			{
				if (cookie[i] == '=')
				{
					key = cookie.substr(start, i - start);
					start = ++i;
					while (i < cookie.length() && cookie[i] != ';')
						i++;
					value = cookie.substr(start, i - start);
					req.setCookies(key, value);
					std::cout << cookie.substr(i) << std::endl;
					if (cookie[i] == ';')
						start = ++i;
				}
			}
			break;
		}
	}
}

// [KEY][=]["][VALUE][""][;][SP][KEY][=]["][VALUE][""][;]
int Parser::fileHeaderParametrs(const std::string &headers, struct File &file, unsigned int i)
{
	std::string key;
	std::string value;
	unsigned int start = i;
	unsigned int oldI = i;

	while (i < headers.length())
	{
		if (headers[++i] == '=') // [=]
		{
			key = headers.substr(start, i - start); // [KEY]
			if (headers[++i] != '"') // ["]
				return (0);

			start = ++i; // skip '"'
			while (i < headers.length() && headers[i] != '"')
				i++;
			value = headers.substr(start, i - start); // [VALUE]
			if (headers[i++] != '"') // ["]
				return (0);

			file.headers.insert(std::make_pair(key, value));
			if (headers[i] == ';') // [;] [SP]
				i += 2;
			else
				return (i);
			start = i;
		}
	}
	return (oldI);
}

// [KEY][:][SP][VALUE][;][SP][KEY][:][SP][VALUE][CRLF][CRLF]
// or [KEY][:][SP][VALUE][;][SP][KEY][=]["][VALUE][""][;]
bool Parser::saveFileHeaders(const std::string &headers, HTTPRequest &req, unsigned int &i)
{
	struct File file;
	std::string key;
	std::string value;
	unsigned int start = 0;

	while (i < headers.length())
	{
		start = i;
		while (i < headers.length() && headers[i] != ':')
			i++;
		key = headers.substr(start, i - start); // [KEY]
		if (headers[i++] != ':') // [:]
			return (false);

		if (headers[i++] != ' ') // [SP]
			return (false);

		start = i;
		while (i < headers.length() && headers[i] != ';' && !hasCRLF(headers.c_str(), i, 0))
			i++;
		value = headers.substr(start, i - start); // [VALUE]

		file.headers.insert(std::make_pair(key, value));
		if (hasCRLF(headers.c_str(), i, 1)) // [CRLF] [CRLF]
			break;

		// parametrs after header
		if (headers[i] == ';' && headers[i + 1] == ' ') // [;]
		{
			i += 2; // skip ; and ' '
			i = fileHeaderParametrs(headers, file, i);
			if (i == 0)
				return (false);
			if (hasCRLF(headers.c_str(), i, 1)) // [CRLF] [CRLF]
				break;
		}
		if (!hasCRLF(headers.c_str(), i, 0)) // each header ends with \r\n
			return (false);
		i += 2; // skip [CRLF]
	}
	i += 2; // skip [CRLF]
	req.setFiles(file);
	return (true);
}

bool Parser::saveFile(const std::string& data, HTTPRequest &req)
{
	unsigned int start;
	unsigned int i = 0;
	
	if (!saveFileHeaders(data, req, i))
		return (false);
	i += 2;
	
	start = i;
	while (i < data.length())
		i++;
	req.setFileContent(data.substr(start, i - start));
	return (true);
}

std::string Parser::extractUploadBoundary(std::string line)
{
	unsigned int start = 0;

	for (unsigned int i = 0; i < line.length(); ++i)
	{
		if (line[i] == '=')
		{
			start = ++i;
			while (i < line.length())
				i++;
			return (line.substr(start, i - start));
		}
	}
	return ("");
}

bool Parser::isUploadBoundary(const std::string &data, HTTPRequest &req, unsigned int &i)
{
	unsigned int start = i;

	while (i < data.length() && !hasCRLF(data.c_str(), i, 0))
		i++;
	if (!hasCRLF(data.c_str(), i, 0))
		return (false);
	if (data.substr(start, i - start) != "--" + req.getUploadBoundary()) // [BOUNDARY]
		return (false);
	i += 2; // [CRLF]
	return (true);
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
if (type == "text/plain" || type == "text/html" || \
	type.substr(0, 30) == "multipart/form-data; boundary=" \
	|| type == "application/octet-stream")
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
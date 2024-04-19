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

// ----------------UTILS----------------------------

bool HTTPRequest::hasMandatoryHeaders(HTTPRequest &obj)
{
	int isHost = 0;
	int isContentLength = 0;
	int isContentType = 0;
	std::multimap<std::string, std::string> headers = obj.getHeaders();
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
			if (!isNumber(it->second) || obj.getMethod() != "POST")
				return (false);
			isContentLength++;
		}
		else if (it->first == "content-type")
		{
			if (!isValidContentType(it->second) || obj.getMethod() != "POST")
				return (false);
			isContentType++;
		}
		else if (it->first == "transfer-encoding")
		{
			if (it->second != "chunked" || obj.getMethod() != "POST")
				return (false);
			obj.setIsChunked(true);
		}
	}
	if (obj.getIsChunked() && isContentLength > 0)
		return (false);
	if (obj.getMethod() == "POST" || obj.getMethod() == "DELETE")
		return (isHost == 1 && isContentLength == 1 && isContentType == 1);
	else
		return (isHost == 1);
}

bool HTTPRequest::saveVariables(std::string &variables)
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
			_queryString.insert(std::make_pair(key, value));
		}
	}
	return (true);
}

void HTTPRequest::makeHeadersLowCase()
{
	std::multimap<std::string, std::string>::iterator it;
	std::multimap<std::string, std::string> newHeaders;
	std::string tmp;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		tmp = it->first;
		for (unsigned int i = 0; i < tmp.size(); ++i)
		{
			if (tmp[i] >= 65 && tmp[i] <= 90)
				tmp[i] = tmp[i] + 32;
		}
		newHeaders.insert(std::make_pair(tmp, it->second));
	}
	_headers.swap(newHeaders);
}

std::string HTTPRequest::extractValue(std::string &variables, int &i)
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

std::string HTTPRequest::extractKey(std::string &variables, int &i, int startPos)
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

std::string HTTPRequest::extractRequestTarget(const char *request, unsigned int &i)
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

std::string HTTPRequest::extractVariables(std::string &requestTarget, bool &isOriginForm)
{
	int queryStart = 0;

	if (requestTarget == "/")
		return ("/");
	isOriginForm = isOrigForm(requestTarget, queryStart);
	if (isOriginForm)
		return (requestTarget.substr(queryStart + 1, strlen(requestTarget.c_str()) - queryStart));
	return (requestTarget);
}

std::string HTTPRequest::extractProtocolVersion(const char *request, unsigned int &i)
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

std::string HTTPRequest::extractMethod(const char *request, unsigned int &i)
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

std::string HTTPRequest::extractHeaderKey(const char *request, unsigned int &i)
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

std::string HTTPRequest::extractHeaderValue(const char *request, unsigned int &i)
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

unsigned int HTTPRequest::extractLineLength(const char *request, unsigned int &i)
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

std::string HTTPRequest::extractLine(const char *request, unsigned int &i, const unsigned int &size)
{
	std::string string_request(request);
	std::string line = string_request.substr(i, size);
	i += size; // skip line
	if (request[i] != '\r' || request[i + 1] != '\n')
		return ("");
	i += 2; // skip '\r' and '\n'
	return (line);
}

bool HTTPRequest::isOrigForm(std::string &requestTarget, int &queryStart)
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

bool HTTPRequest::isValidContentType(std::string type)
{
	if (type == "text/plain" || type == "text/html")
		return (true);
	return (false);
}

bool HTTPRequest::isValidHost(std::string host)
{
	(void)host;
	return (true);
}

void HTTPRequest::skipRequestLine(const char *request, unsigned int &i)
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

void HTTPRequest::skipHeader(const char *request, unsigned int &i)
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
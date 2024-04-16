#include "HTTPRequest.hpp"
#include "webserv.hpp"

HTTPRequest::HTTPRequest() : _statusCode(200), _isChunked(false), _method(""), _requestTarget(""), _protocolVersion(""), _uploadBoundary(""), _errorMessage("")
{
}

HTTPRequest::HTTPRequest(const HTTPRequest &obj)
{
	_statusCode = obj._statusCode;
	_isChunked = obj._isChunked;
	_method = obj._method;
	_requestTarget = obj._requestTarget;
	_protocolVersion = obj._protocolVersion;
	_body = obj._body;
	_headers = obj._headers;
	_queryString = obj._queryString;
	_uploadBoundary = obj._uploadBoundary;
	_errorMessage = obj._errorMessage;
	_isChunkFinish = obj._isChunkFinish;
}

HTTPRequest &HTTPRequest::operator=(const HTTPRequest &obj)
{
	if (this != &obj)
	{
		_statusCode = obj._statusCode;
		_isChunked = obj._isChunked;
		_method = obj._method;
		_requestTarget = obj._requestTarget;
		_protocolVersion = obj._protocolVersion;
		_body = obj._body;
		_headers = obj._headers;
		_queryString = obj._queryString;
		_uploadBoundary = obj._uploadBoundary;
		_errorMessage = obj._errorMessage;
		_isChunkFinish = obj._isChunkFinish;
	}
	return (*this);
}

HTTPRequest::~HTTPRequest()
{
}

HTTPRequest::HTTPRequest(const char *request)
{
	_statusCode = 200;
	if (strlen(request) < 10)
		ft_error(400, "Invalid request-line");
	else
	{
		parseRequestLine(request);
		if (_statusCode == 200)
			parseHeaders(request);
		if (_statusCode == 200 && !_isChunked && _method != "GET" && _uploadBoundary.empty())
			parseBody(request);
		else if (_statusCode == 200 && !_isChunked && _method != "GET")
			parseFileBody(request);
		std::vector<File>::iterator it;
		std::map<std::string, std::string>::iterator it2;
		std::vector<std::string>::iterator it3;
		for (it = _files.begin(); it != _files.end(); ++it){
			for (it2 = it->headers.begin(); it2 != it->headers.end(); ++it2)
				std::cout << "Key: " << it2->first << ", Value: " << it2->second << std::endl;
			for (it3 = it->fileContent.begin(); it3 != it->fileContent.end(); ++it3)
				std::cout << "Data: " << *it3 << std::endl;
		}
	}
}

std::string HTTPRequest::getMethod() const
{
	return (_method);
}

std::string HTTPRequest::getProtocolVersion() const
{
	return (_protocolVersion);
}

std::string HTTPRequest::getRequestTarget() const
{
	return (_requestTarget);
}

int HTTPRequest::getStatusCode() const
{
	return (_statusCode);
}

bool HTTPRequest::getIsChunked() const
{
	return (_isChunked);
}

std::string HTTPRequest::getHost() const
{
	std::multimap<std::string, std::string>::const_iterator it = _headers.find("Host");
	if (it != _headers.end())
	{
		return it->second;
	}
	return "";
}

std::multimap<std::string, std::string> HTTPRequest::getQueryString() const
{
	return (_queryString);
}

std::multimap<std::string, std::string> HTTPRequest::getHeaders() const
{
	return (_headers);
}

std::pair<std::string, std::string> HTTPRequest::getHeaders(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

bool HTTPRequest::getIsChunkFinish() const
{
	return (_isChunkFinish);
}

std::vector<std::string> HTTPRequest::getBody() const
{
	return (_body);
}

std::string HTTPRequest::getErrorMessage() const
{
	return (_errorMessage);
}

std::string HTTPRequest::getUploadBoundary() const
{
	return (_uploadBoundary);
}

// std::vector<File> HTTPRequest::getFiles() const
// {
// 	return (_files);
// }

void HTTPRequest::setIsChunked(bool n)
{
	_isChunked = n;
}


bool HTTPRequest::ft_error(int statusCode, std::string message)
{
	_errorMessage = message;
	_statusCode = statusCode;
	return (false);
}

bool HTTPRequest::parseRequestLine(const char *request)
{
	unsigned int i = 0;
	bool isOriginForm = false;
	std::string variables;

	_method = extractMethod(request, i);
	if (_method.empty())
		return (ft_error(501, "Invalid method"));
	if (request[i++] != ' ')
		return (ft_error(400, "Invalid request-line syntax"));
	_requestTarget = extractRequestTarget(request, i);
	if (_requestTarget.empty())
		return (ft_error(414, "Request-target is too long"));
	if (request[i++] != ' ')
		return (ft_error(400, "Invalid request-line syntax"));
	variables = extractVariables(_requestTarget, isOriginForm);
	if (variables.empty())
		return (ft_error(400, "Invalid query"));
	if (isOriginForm)
		if (!saveVariables(variables))
			return (ft_error(400, "Invalid query"));
	_protocolVersion = extractProtocolVersion(request, i);
	if (_protocolVersion.empty())
		return (ft_error(400, "Invalid protocol"));
	if (!hasCRLF(request, i, 0))
		return (ft_error(400, "Invalid CRLF for request-line"));
	return (true);
}

bool HTTPRequest::parseChunkedBody(const char *request)
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
				return (ft_error(400, "Invalid body"));
			break;
		}
		line = extractLine(request, i, size);
		if (line.empty())
			return (ft_error(400, "Invalid body"));
		_body.push_back(line);
	}
	if (!hasCRLF(request, i, 0))
		return (ft_error(400, "Invalid body"));
	if (hasCRLF(request, i, 1))
		_isChunkFinish = true;
	return (true);
}

bool HTTPRequest::parseHeaders(const char *request)
{
	unsigned int i;
	std::string key;
	std::string value;

	i = 0;
	skipRequestLine(request, i);
	while (request[i])
	{
		key = extractHeaderKey(request, i);
		if (key.empty())
			return (ft_error(400, "Invalid header key"));
		i++; // skip ':'
		if (request[i++] != ' ')
			return (ft_error(400, "Invalid header syntax"));
		value = extractHeaderValue(request, i);
		if (value.empty())
			return (ft_error(400, "Invalid header value"));
		if (!hasCRLF(request, i, 0))
		if (!hasCRLF(request, i, 0))
			return (ft_error(400, "No CRLF after header"));
		_headers.insert(std::make_pair(key, value));
		i += 2;						// skip '\r' and '\n'
		if (hasCRLF(request, i, 0)) // end of header section
			break;
	}
	if (!hasCRLF(request, i, 0))
	if (!hasCRLF(request, i, 0))
		return (ft_error(400, "No CRLF after header"));
	makeHeadersLowCase();
	if (!hasMandatoryHeaders())
		return (400);
	if (_method == "GET" && request[i + 2]) // has something after headers
		return (ft_error(400, "Invalid headers"));
	return (true);
}

bool HTTPRequest::parseBody(const char *request)
{
	bool end = false;
	unsigned int i = 0;
	unsigned int start = 0;
	std::string string_request(request);

	skipHeader(request, i);
	start = i;
	while (request[i] && !end)
	{
		if (hasCRLF(request, i, 0))
		{
			if (hasCRLF(request, i, 1))
				end = true;
			_body.push_back(string_request.substr(start, i - start));
			i += 2;
			start = i;
			continue;
		}
		else if (!hasCRLF(request, i, 0) && request[i] == '\r')
			return (ft_error(400, "Invalid body"));
		i++;
	}
	if (end && hasCRLF(request, i, 0) && request[i + 2])
		return (ft_error(400, "Invalid body"));
	return (end);
}

// [BOUNDARY][CRLF][HEADERS][CRLF][DATA][CRLF][BOUNDARY--][CRLF][CRLF]
bool HTTPRequest::parseFileBody(const char *request)
{
	unsigned int i = 0;
	unsigned int start = 0;
	bool isFinish = false;

	skipHeader(request, i);
	start = i;
	while (request[i])
	{
		if (start == i && !isUploadBoundary(request, i)) // [BOUNDARY] [CRLF]
			return (ft_error(400, "Invalid boundary in the file upload"));
		if (!saveFileHeaders(request, i)) // [HEADERS] [CRLF]
			return (ft_error(400, "Invalid file upload headers"));
		if (!saveFileData(request, i, isFinish)) // [DATA] [BOUNDARY--]
			return (ft_error(400, "Invalid file data"));
		if (isFinish)
			break ;
	}
	if (isFinish && hasCRLF(request, i, 0) && !request[i + 2]) // nothing after body
		return (true);
	return (ft_error(400, "Invalid file upload body"));
}

std::ostream& operator<<(std::ostream& out, const HTTPRequest& obj)
{
	std::multimap<std::string, std::string> a = obj.getHeaders();
	std::multimap<std::string, std::string> b = obj.getQueryString();
	std::vector<std::string>				c = obj.getBody();

	std::multimap<std::string, std::string>::iterator it;
	out << "---------------------Variables--------------------" << std::endl;
	for (it = b.begin(); it != b.end(); it++){
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	out << "---------------------End--------------------------" << std::endl;
	out << "---------------------Headers----------------------" << std::endl;
	for (it = a.begin(); it != a.end(); it++){
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	out << "---------------------End--------------------------" << std::endl;
	out << "---------------------Body-------------------------" << std::endl;
	for (size_t i = 0; i < c.size(); ++i)
		out << c[i] << std::endl;
	out << "---------------------End--------------------------" << std::endl;

	// for (unsigned int i = 0; i <)
	return (out);
}








// -----------------------UTILS-------------------------------










bool HTTPRequest::hasMandatoryHeaders()
{
	int isHost = 0;
	int isContentLength = 0;
	int isContentType = 0;
	std::multimap<std::string, std::string> headers = _headers;
	std::multimap<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++)
	{
		if (it->first == "host")
		{
			if (!isValidHost(it->second))
				return (ft_error(400, "Invalid host"));
			isHost++;
		}
		else if (it->first == "content-length")
		{
			if (!isNumber(it->second) || _method != "POST")
				return (ft_error(400, "Invalid content-length"));
			isContentLength++;
		}
		else if (it->first == "content-type")
		{
			if (!isValidContentType(it->second) || _method != "POST")
				return (ft_error(400, "Invalid content-type"));
			if (it->second.substr(0, 30) == "multipart/form-data; boundary=")
				_uploadBoundary = extractUploadBoundary(it->second);
			isContentType++;
		}
		else if (it->first == "transfer-encoding")
		{
			if (it->second != "chunked" || _method != "POST")
				return (ft_error(400, "Invalid transfer-encoding"));
			_isChunked = true;
		}
	}
	if (_isChunked && isContentLength > 0)
		return (ft_error(400, "Invalid chunked request"));
	if (_method == "POST" || _method == "DELETE"){
		if (!(isHost == 1 && isContentLength == 1 && isContentType == 1))
			return (ft_error(400, "Invalid headers"));
	}
	else
		if (isHost != 1)
			return (ft_error(400, "Request MUST have host"));
	return (true);
}

// [KEY][:][SP][VALUE][;][SP][KEY][:][SP][VALUE][CRLF][CRLF]
bool HTTPRequest::saveFileHeaders(const std::string& headers, unsigned int& i)
{
	struct File file;
	std::string key;
	std::string value;
	unsigned int start = 0;

	while (i < headers.length()){
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
			break ;
		if (headers[i] && headers[i++] != ';') // [;]
			return (false);
		if (headers[i] && headers[i++] != ' ') // [SP]
			return (false);
	}
	if (!hasCRLF(headers.c_str(), i, 1)) // [CRLF] [CRLF]
		return (false);
	_files.push_back(file);
	i += 4; // skip [CRLF] [CRLF]
	return (true);
}

// [DATA][CRLF][DATA[CRLF][BOUNDARY]
bool HTTPRequest::saveFileData(const std::string& data, unsigned int& i, bool& isFinish)
{
	std::vector<std::string> tmpArray;
	unsigned int start = 0;

	while (i < data.length()){
		start = i;
		while (i < data.length() && !hasCRLF(data.c_str(), i, 0))
			i++;
		if (!hasCRLF(data.c_str(), i, 0)) // [CRLF]
			return (false);
		if (data.substr(start, i - start) == _uploadBoundary + "--"){ // [BOUNDARY--]
			isFinish = true;
			break ;
		}
		if (data.substr(start, i - start) == _uploadBoundary) // [BOUNDARY]
			break ;
		tmpArray.push_back(data.substr(start, i - start)); // [DATA]
		i += 2; // skip [CRLF]
	}
	_files.back().fileContent = tmpArray;
	i += 2; // skip [CRLF]
	return (true);
}

bool HTTPRequest::isUploadBoundary(const std::string& data, unsigned int& i)
{
	unsigned int start = i;

	while (i < data.length() && !hasCRLF(data.c_str(), i, 0))
		i++;
	if (!hasCRLF(data.c_str(), i, 0))
		return (false);
	if (data.substr(start, i - start) != _uploadBoundary) // [BOUNDARY]
		return (false);
	i += 2; // [CRLF]
	return (true);
}

bool HTTPRequest::saveVariables(std::string& variables)
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

void	HTTPRequest::makeHeadersLowCase()
{
	std::multimap<std::string, std::string>::iterator it;
	std::multimap<std::string, std::string> newHeaders;
	std::string	tmp;

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
	if (type == "text/plain" || type == "text/html" || \
	type.substr(0, 30) == "multipart/form-data; boundary=")
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

std::string HTTPRequest::extractUploadBoundary(std::string line)
{
	unsigned int start = 0;

	for (unsigned int i = 0; i < line.length(); ++i){
		if (line[i] == '='){
			start = ++i;
			while (i < line.length())
				i++;
			return (line.substr(start, i - start));
		}
	}
	return ("");
}

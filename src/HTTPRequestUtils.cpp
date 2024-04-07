#include "HTTPRequest.hpp"

bool	isOrigForm(std::string &requestTarget, int &queryStart){
	for (int i = 0; i < (int)requestTarget.length(); i++){
		if (requestTarget[i] == '?'){
			queryStart = i;
			return (true);
		}
	}
	return (false);
}

bool	fileExists(std::string &requestTarget, bool isOriginForm, int queryStart){
	if (isOriginForm && \
	access(("." + requestTarget.substr(0, queryStart)).c_str(), F_OK) == -1)
		return (false);
	if (!isOriginForm && access(("." + requestTarget).c_str(), F_OK) == -1)
		return (false);
	return (true);
}

bool	isInvalidChar(char c)
{
	if ((c >= 0 && c <= 31) || c == 127)
		return (true);
	return (false);
}

void	skipRequestLine(char *request, int& i)
{
	while (request[i] && request[i + 1]){
		if (request[i] == '\r' && request[i + 1] == '\n'){
			i += 2;
			return ;	
		}
		i++;
	}
}

bool	isValidContentType(std::string type)
{
	if (type == "text/plain" || type == "text/html")
		return (true);
	return (false);
}

bool	isValidHost(std::string host)
{
	(void)host;
	return (true);
}

bool	hasMandatoryHeaders(HTTPRequest& obj)
{
	int	isHost = 0;
	int	isContentLength = 0;
	int	isContentType = 0;
	std::multimap<std::string, std::string> headers = obj.getHeaders();
	std::multimap<std::string, std::string>::iterator it;

	for (it = headers.begin(); it != headers.end(); it++){
		if (it->first == "Host"){
			if (!isValidHost(it->second))
				return (false);
			isHost++;
		}
		else if (it->first == "Content-length")
			isContentLength++;
		else if (it->first == "Content-type"){
			if (!isValidContentType(it->second))
				return (false);
			isContentType++;
		}
		else if (it->first == "Transfer-encoding"){
			if (it->second == "chunked")
				obj.setIsChunked(true);
			else
				return (false);
		}
	}
	if (obj.getMethod() == "POST" || obj.getMethod() == "PUT")
		return (isHost == 1 && isContentLength == 1 && isContentType == 1);
	else
		return (isHost == 1);
}

std::string	extractValue(std::string& variables, int &i)
{
	int	startPos;

	startPos = i++;
	while (variables[i] && variables[i] != '&'){
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

std::string extractKey(std::string& variables, int &i, int startPos)
{
	if (i == 0 || variables[i] == '?')
		return ("");
	if (variables[startPos] == '&' && startPos != 0)
		startPos++;
	if (variables.substr(startPos, i - startPos).find('&') != std::string::npos \
	|| variables.substr(startPos, i - startPos).find('?') != std::string::npos)
		return ("");
	return (variables.substr(startPos, i - startPos));
}

std::string extractRequestTarget(char *request, int &i)
{
	std::string	requestTarget;
	std::string	string_request(request);
	int			start = i;

	while (request[i] && request[i] != ' ')
		i++;
	if (i > MAX_URI)
		return ("");
	requestTarget = string_request.substr(start, i - start);
	//std::cout << requestTarget << std::endl;
	return (requestTarget);
}

std::string extractProtocolVersion(char *request, int &i)
{
	std::string	protocolVersion;
	std::string	string_request(request);
	int			start = i;

	while (request[i] && request[i] != '\r')
		i++;
	protocolVersion = string_request.substr(start, i - start);
	//std::cout << protocolVersion << std::endl;
	if (protocolVersion == "HTTP/1.1")
		return (protocolVersion);
	return ("");
}

std::string	extractMethod(char *request, int &i)
{
	std::string	method;
	std::string	string_request(request);

	while (request[i] && request[i] != ' ')
		i++;
	method = string_request.substr(0, i);
	//std::cout << method << std::endl;
	if (method == "GET" || method == "POST" || method == "DELETE")
		return (method);
	return ("");
}

std::string	extractHeaderKey(char *request, int& i)
{
	std::string	string_request(request);
	int			start = i;

	while (request[i] && request[i] != ':'){
		if (isInvalidChar(request[i]) || request[i] == ' ')
			return ("");
		i++;
	}
	//std::cout << "Key: "<< string_request.substr(start, i - start) << std::endl;
	return (string_request.substr(start, i - start));
}

std::string	extractHeaderValue(char *request, int& i)
{
	std::string string_request(request);
	int			start = i;

	while (request[i] && request[i] != '\r'){
		if (isInvalidChar(request[i]))
			return ("");
		i++;
	}
	//std::cout << "Value: "<< string_request.substr(start, i - start) << std::endl;
	return (string_request.substr(start, i - start));
}

int parseHeaders(char *request, HTTPRequest& obj)
{
	int			i;
	std::string	key;
	std::string	value;

	i = 0;
	skipRequestLine(request, i);
	while (request[i]){
		key = extractHeaderKey(request, i);
		if (key.empty())
			return (400);
		i++; // skip ':'
		if (request[i++] != ' ')
			return (400);
		value = extractHeaderValue(request, i);
		if (value.empty())
			return (400);
		if (request[i] != '\r' || request[i + 1] != '\n')
			return (400);
		obj.addHeader(key, value);
		i += 2; // skip '\r' and '\n'
		if (request[i] == '\r' && request[i + 1] == '\n') // end of header section
			break ;
	}
	if (request[i] != '\r' || request[i + 1] != '\n') // end of header section
		return (400);
	if (!hasMandatoryHeaders(obj))
		return (400);
	if (obj.getMethod() == "GET" && request[i + 2]) //has something after headers
		return (400);
	return (200);
}

int	parseBody(){
	return (0);
}

int	parseChunkedBody(){
	return (0);
}
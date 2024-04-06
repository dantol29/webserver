#include "HTTPRequest.hpp"

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
	if (i == 0)
		return ("");
	if (variables[startPos] == '&')
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

bool isOrigForm(std::string &requestTarget, int &queryStart){
	for (int i = 0; i < (int)requestTarget.length(); i++){
		if (requestTarget[i] == '?'){
			queryStart = i;
			return (true);
		}
	}
	return (false);
}

bool fileExists(std::string &requestTarget, bool isOriginForm, int queryStart){
	if (isOriginForm && \
	access(("." + requestTarget.substr(0, queryStart)).c_str(), F_OK) == -1)
		return (false);
	if (!isOriginForm && access(("." + requestTarget).c_str(), F_OK) == -1)
		return (false);
	return (true);
}

#include <string>
#include <iostream>
#include <unistd.h>
# define MAX_URI 100

std::string	extractMethod(char *request, int &i)
{
	std::string	method;
	std::string	string_request(request);

	while (request[i] && request[i] != ' ')
		i++;
	method = string_request.substr(0, i);
	std::cout << method << std::endl;
	if (method == "GET" || method == "POST" || method == "DELETE")
		return (method);
	return (NULL);
}

std::string extractRequestTarget(char *request, int &i)
{
	std::string	requestTarget;
	std::string	string_request(request);
	int			start = i;

	while (request[i] && request[i] != ' ')
		i++;
	if (MAX_URI > 100)
		return (NULL);
	requestTarget = string_request.substr(start, i - start);
	std::cout << requestTarget << std::endl;
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
	std::cout << protocolVersion << std::endl;
	if (protocolVersion == "HTTP/1.1")
		return (protocolVersion);
	return (NULL);
}

bool	checkRequestTarget(std::string requestTarget)
{
	if (requestTarget == "/")
		return (true);
	std::cout << "." + requestTarget << std::endl;
	if (access(("." + requestTarget).c_str(), F_OK) == -1)
		return (false);
	return (true);
}

/*
method token(GET, POST...), single space, request target, single space, protocol version
*/
int	parseRequestLine(char *request)
{
	int			i;
	std::string	method;
	std::string	requestTarget;
	std::string	protocolVersion;

	i = 0;
	method = extractMethod(request, i);
	if (method.empty()) // A server that receives a method longer than any that it implements
		return (501); // SHOULD respond with a 501 (Not Implemented).
	if (request[i++] != ' ') // single space
		return (400);
	requestTarget = extractRequestTarget(request, i);
	if (requestTarget.empty()) // A server that receives a request-target longer 
		return (414); // than any URI it wishes to parse MUST respond with a 414 (URI Too Long).
	if (request[i++] != ' ') // single space
		return (400);
	if (!checkRequestTarget(requestTarget))
		return (400); // An invalid request-line SHOULD respond with a 400 (Bad Request).
	protocolVersion = extractProtocolVersion(request, i);
	if (protocolVersion.empty())
		return (400);
	if (request[i] != '\r' || !request[i + 1] || request[i + 1] != '\n') // CRLF
		return (400); // The combination of \r\n serves as a standard way to denote the end of a line in HTTP headers.
	return (200); 
}

int	parseHttpRequest(char *request)
{
	return (parseRequestLine(request));
}
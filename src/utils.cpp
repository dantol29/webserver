#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include "webserv.hpp"

std::string readHtml(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
	{
		std::cerr << "Could not open file: " << filePath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

char *ft_strcpy(char *dest, const char *src)
{
	int i;

	i = 0;
	if (!dest || !src)
		return (NULL);
	while (dest[i] && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

int hexToInt(std::string hex)
{
	int n;

	std::stringstream ss;
	ss << std::hex << hex;
	ss >> n;
	if (ss.fail())
		return (-1);
	return (n);
}

bool	isNumber(std::string line)
{
	for (unsigned int i = 0; i < line.length(); ++i){
		if (!(line[i] >= '0' && line[i] <= '9'))
			return (false);
	}
	return (true);
}

/*
The combination of \\r\\n
is as a standard way to denote the end of a line in HTTP headers.
*/
bool hasCRLF(const char *request, unsigned int &i, int mode)
{
	if (mode == 0)
	{
		if (request[i] && request[i + 1] && request[i] == '\r' && request[i + 1] == '\n')
			return (true);
		return (false);
	}
	if (mode == 1)
	{
		if (request[i] && request[i + 1] && request[i + 2] && request[i + 3] && request[i] == '\r' &&
			request[i + 1] == '\n' && request[i + 2] == '\r' && request[i + 3] == '\n')
			return (true);
		return (false);
	}
	return (false);
}

bool isInvalidChar(const unsigned char &c)
{
	if ((c <= 31) || c == 127)
		return (true);
	return (false);
}
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
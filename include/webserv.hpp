#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include "MetaVariables.hpp"
#include "HTTPRequest.hpp"
#include "Parser.hpp"

template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

bool startsWith(const std::string &fullString, const std::string &starting);
std::string readHtml(const std::string &filePath);

// utils.cpp
int hexToInt(std::string hex);
bool isNumber(std::string line);
bool isInvalidChar(const unsigned char &c);
bool hasCRLF(const char *request, unsigned int &i, int mode);
bool isVulnerablePath(const std::string &path);
int checkFile(const char *path);

#endif
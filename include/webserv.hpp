#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

// for file uploads
struct File
{
	std::map<std::string, std::string> headers;
	std::string fileContent;
};

bool startsWith(const std::string &fullString, const std::string &starting);
std::string readHtml(const std::string &filePath);
void printHTTPRequest(const std::string httpRequest, size_t startPos = 0);

// utils.cpp
int hexToInt(std::string hex);
bool isNumber(std::string line);
bool isInvalidChar(const unsigned char &c);
bool hasCRLF(const char *request, unsigned int &i, int mode);
bool isVulnerablePath(const std::string &path);
int checkFile(const char *path);
int strToInt(const std::string &str);

#endif
#ifndef WEBSERV_H
#define WEBSERV_H

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include "Debug.hpp"

#define SEND_BUFFER_SIZE 1024 * 100 // 100 KB
#define BUFFER_SIZE 1025
#define CGI_BUFFER_SIZE 100 // 100 B
#define CGI_TIMEOUT_S 6 // 6 seconds
#define CGI_POLL_TIMEOUT_MS 500 // 0.5 seconds
#define CLIENT_POLL_TIMEOUT_MS 12000 // 12 seconds
#define CLIENT_TIMEOUT_S 10 // 10 seconds
#define CONFIG_FILE_DEFAULT_PATH "./conf/webserv_default.conf"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define WHITE "\033[1;37m"
#define PURPLE "\033[1;35m"
#define ORANGE "\033[38;5;208m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define BLINKING "\033[5m"
#define RESET "\033[0m"

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

void test_debug();

bool startsWith(const std::string &fullString, const std::string &starting);
std::string readHtml(const std::string &filePath);
void printHTTPRequest(const std::string httpRequest, size_t startPos = 0);

// utils.cpp
int hexToInt(std::string hex);
bool isNumber(std::string line);
bool isInvalidChar(const unsigned char &c);
bool hasCRLF(const char *request, unsigned int &i, int mode);
bool isVulnerablePath(const std::string &path);
bool isValidErrorCode(std::string errorCode);
int checkFile(const char *path);
int strToInt(const std::string &str);

#endif
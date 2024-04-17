#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>
#include <sstream>
#include <cstdlib>
#include <string.h>
#include "MetaVariables.hpp"
#include "HTTPRequest.hpp"

// #include "MetaVariables.hpp"
// #include "HTTPRequest.hpp"
// #include "CGIHandler.hpp"
// #include "StaticContentHandler.hpp"
// #include "Router.hpp"
// #include "HTTPResponse.hpp"
// #include "server_utils.hpp"

// declared here because it is a template function
// instead of to_string which is c++11, we use this function
template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

char *ft_strcpy(char *dest, const char *src);
bool startsWith(const std::string &fullString, const std::string &starting);

/**
 * Reads the entire content of an HTML file into a string.
 * This function opens the specified file, reads its contents into a string,
 * and then returns that string. If the file cannot be opened, it prints an error
 * message to standard error and returns an empty string.
 * @param filePath The relative or absolute path to the HTML file.
 * @return A string containing the HTML content of the file, or an empty string if the file cannot be opened.
 */
std::string readHtml(const std::string &filePath);

/**
 * Constructs an HTTP response containing the HTML content for the home page.
 * This function reads the home page HTML file using readHtml() and then wraps it
 * in a standard HTTP response format, including headers for content type and length.
 * @return An HTTP response string including headers and the HTML content of the home page.
 */
std::string handleHomePage();

/**
 * Generates a standard HTTP 404 Not Found response.
 * This function creates an HTTP response indicating that the requested resource was not found.
 * It includes headers specifying that no content is being returned.
 * @return An HTTP 404 Not Found response string.
 */
// std::string handleNotFound(void);

/**
 * Reads an HTTP request from a socket, determines the type of request,
 * and dispatches it to the appropriate handler function.
 * This function reads the request from the socket into a buffer, identifies the request type,
 * and then calls one of handleHomePage(), handleCGIRequest(), or handleNotFound() to generate the response.
 * The response is then written back to the socket.
 * @param socket The socket descriptor representing the connection from a client.
 */
// void handleConnection(int socket);

// utils.cpp
int hexToInt(std::string hex);
bool isNumber(std::string line);
bool isInvalidChar(const unsigned char &c);
bool hasCRLF(const char *request, unsigned int &i, int mode);
bool isVulnerablePath(const std::string &path);
int checkFile(const char *path);

#endif
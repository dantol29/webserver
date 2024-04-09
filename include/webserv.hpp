#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>
#include "../src/Environment.hpp"
#include <sstream>

// declared here because it is a template function
// instead of to_string which is c++11, we use this function
template <typename T>
std::string toString(const T &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void *ft_memset(void *ptr, int value, size_t num);
char *ft_strcpy(char *dest, const char *src);

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
// std::string handleHomePage();

/**
 * Handles the execution of a CGI script and captures its output to construct an HTTP response.
 * This function sets up a pipe for capturing the standard output of the CGI script, forks the process,
 * and executes the script in the child process using the provided environment settings. The parent process
 * reads the script's output from the pipe, waits for the script to finish execution, and then constructs
 * an HTTP response with the script's output. If an error occurs during the process, a 500 Internal Server Error
 * response is generated instead.
 *
 * @param argv An array of char pointers representing the arguments to be passed to the CGI script,
 *             including the script path as the first argument.
 * @param env An Environment object containing the environment variables to be passed to the CGI script.
 * @return A string representing the HTTP response generated from the CGI script's output, or a 500 Internal
 *         Server Error message if the script execution fails.
 */
std::string handleCGIRequest(const char *argv[], Environment env);

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
void handleConnection(int socket);

#endif
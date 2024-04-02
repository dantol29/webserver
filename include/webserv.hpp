#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>


/**
 * Reads the entire content of an HTML file into a string.
 * This function opens the specified file, reads its contents into a string,
 * and then returns that string. If the file cannot be opened, it prints an error
 * message to standard error and returns an empty string.
 * @param filePath The relative or absolute path to the HTML file.
 * @return A string containing the HTML content of the file, or an empty string if the file cannot be opened.
 */
std::string readHtml(const std::string& filePath);

/**
 * Constructs an HTTP response containing the HTML content for the home page.
 * This function reads the home page HTML file using readHtml() and then wraps it
 * in a standard HTTP response format, including headers for content type and length.
 * @return An HTTP response string including headers and the HTML content of the home page.
 */
std::string handleHomePage();

/**
 * Handles the execution of a CGI script and captures its output to construct an HTTP response.
 * This function sets up a pipe for capturing the standard output of a CGI script, forks the process,
 * and executes the script in the child process. The parent process reads the script's output from the pipe,
 * waits for the script to finish execution, and then constructs an HTTP response with the script's output.
 * @param argv The arguments to be passed to the CGI script, including the script path as the first argument.
 * @param envp The environment variables to be passed to the CGI script.
 * @return An HTTP response string containing the output of the CGI script execution, or a 500 Internal Server Error if an error occurs.
 */
std::string handleCGIRequest(const char* argv[], const char* envp[]);

/**
 * Generates a standard HTTP 404 Not Found response.
 * This function creates an HTTP response indicating that the requested resource was not found.
 * It includes headers specifying that no content is being returned.
 * @return An HTTP 404 Not Found response string.
 */
std::string handleNotFound(void);

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
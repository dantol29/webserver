#ifndef WEBSERV_H
#define WEBSERV_H

#include <string>

/**
 * @brief Reads the entire content of a file into a string.
 *
 * This function opens the file specified by `filePath` 
 * and reads its entire content into a single string. 
 * It's designed to handle text files. 
 * If the file cannot be opened, the function outputs an error message to `std::cerr` 
 * and returns an empty string.
 *
 * @param filePath A constant reference to a `std::string` that contains the path to the file to be read.
 * @return A `std::string` containing the contents of the file. 
 * Returns an empty string if the file cannot be opened or is otherwise unreadable.
 *
 * @note This function uses `std::ifstream` to open and read the file. 
 * It assumes the file's contents are text and can be represented as a string. 
 * Binary files or files with data not compatible with `std::string` may not be handled correctly.
 */
std::string readFile(const std::string& filePath);

/**
 * Sends the home page content to the client.
 * 
 * This function reads the HTML content from a file named "home.html" located in the current directory.
 * Then constructs an HTTP response including the status line, headers, and the HTML content.
 * Finally, it sends this response back to the client through the specified socket.
 *
 * @param socket The socket descriptor through which the home page content will be sent to the client.
 */
void handleHomePage(int socket);

/**
 * Executes a CGI script and sends its output to the client.
 * 
 * This function runs a CGI script located at "./cgi-bin/hello.cgi"
 * with `fork()`, `pipe()`, and `execve()` to handle the execution. 
 * It captures the script's output through a pipe, and send it back to the client.
 *
 * NOTE AND FUTURE IMPROVEMENT: The function assumes the existence of the script at the specified location 
 * and does not add additional HTTP headers to the output. 
 * It's designed to demonstrate CGI integration rather than a fully-featured HTTP response handling.
 *
 * @param socket The socket descriptor through which the CGI script's output will be sent to the client.
 */
void handleHelloPage(int socket);


/**
 * Sends a 404 Not Found response to the client.
 * 
 * This function constructs a simple HTTP 404 Not Found response with no body.
 * It is used to handle requests for resources that are not available or not recognized
 * by the server. The response is sent back to the client through the specified socket.
 *
 * @param socket The socket descriptor through which the 404 response will be sent to the client.
 */
void handleNotFound(int socket);

/**
 * Handles incoming connections and dispatches them to appropriate handlers.
 * 
 * This function reads the HTTP request from the client and determines the type of request.
 * Depending on the request path, it calls either `handleHomePage` to serve the home page,
 * `handleHelloPage` to execute and return the output of a CGI script, or `handleNotFound`
 * if the requested path is not recognized. After processing the request, it closes the socket.
 *
 * @param socket The socket descriptor representing the client connection to be handled.
 */
void handleConnection(int socket);
#endif
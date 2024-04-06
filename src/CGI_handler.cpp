#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"
#include "HTTPRequest.hpp"
#include <sys/wait.h>

std::string handleCGIRequest(const char* argv[], Environment env) {

    std::vector<char*> envp = env.getForExecve();

    int pipeFD[2];
    if (pipe(pipeFD) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipeFD[0]);
        dup2(pipeFD[1], STDOUT_FILENO);
        close(pipeFD[1]);

        if (!envp.empty()) {
        // Convert the vector to a suitable format for execve
            if (execve(argv[0], const_cast<char* const*>(argv), &envp[0]) == -1) {
                perror("execve failed");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        close(pipeFD[1]);

        std::string cgiOutput;
        char readBuffer[256];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeFD[0], readBuffer, sizeof(readBuffer) - 1)) > 0) {
            readBuffer[bytesRead] = '\0';
            cgiOutput += readBuffer;
        }
        close(pipeFD[0]);

        int status;
        waitpid(pid, &status, 0);

        // Construct the HTTP response
        std::string httpResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n" +
                        std::string("Content-Length: ") + toString(cgiOutput.length()) + "\n\n" + cgiOutput;
        
        std::cout << "------------------CGI output prepared-------------------" << std::endl;
        return httpResponse;
    }

    //line necessary to compile
    return "HTTP/1.1 500 Internal Server Error\nContent-Length: 0\n\n";
}

//refer to RFC 3875 for more information on CGI environment variables
void HTTPRequestToMetaVars(char* rawRequest, Environment& env) {
    HTTPRequest request(rawRequest);

    // Set the method used for the request (e.g., GET, POST)
    env.setVar("REQUEST_METHOD", request.getMethod());
    // Set the protocol version used in the request (e.g., HTTP/1.1)
    env.setVar("PROTOCOL_VERSION", request.getProtocolVersion());

    // Server-related variables
    // The name and version of the HTTP server (Format: name/version)
    env.setVar("SERVER_SOFTWARE", "Server_of_people_identifying_as_objects/1.0");
    // The host name, DNS alias, or IP address of the server
    env.setVar("SERVER_NAME", "The_Objects.com");
    // The CGI specification revision the server is using (Format: CGI/version)
    env.setVar("GATEWAY_INTERFACE", "CGI/1.1");

    // Request-specific variables
    // The name and revision of the protocol the request was made in (Format: protocol/revision)
    env.setVar("SERVER_PROTOCOL", request.getProtocolVersion());
    // The port number on which the request was received
    env.setVar("SERVER_PORT", "8080"); //     ---> how do I set it programmatically ?
    // Additional path information from the client's request URL
    env.setVar("PATH_INFO", ""); 
    // The translated physical path the request refers to (after virtual to physical conversion by the server)
    env.setVar("PATH_TRANSLATED", ""); // Needs specific server-side logic to determine
    // The virtual path to the script being executed
    env.setVar("SCRIPT_NAME", ""); // Needs to be set based on the script's location
    // The query string from the URL sent by the client
    env.setVar("QUERY_STRING", ""); // Needs parsing from the request
    // The host name of the client making the request
    env.setVar("REMOTE_HOST", ""); // Might require reverse DNS lookup
    // The IP address of the client
    env.setVar("REMOTE_ADDR", ""); // Needs to be obtained from the request/connection
    // The authentication method used to protect the script
    env.setVar("AUTH_TYPE", ""); // Depends on server configuration
    // The client's username, if the script is protected and the server supports authentication
    env.setVar("REMOTE_USER", ""); // Depends on server and authentication method
    // The remote (client's) username from RFC 931 identification; for log purposes only
    env.setVar("REMOTE_IDENT", ""); // Requires specific server support
    // The content type attached to the request, if any
    env.setVar("CONTENT_TYPE", ""); // Needs to be parsed from the request headers
    // The length of the content sent by the client
    env.setVar("CONTENT_LENGTH", ""); // Needs to be parsed from the request headers
}

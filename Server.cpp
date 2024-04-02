#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"

const int BUFFER_SIZE = 1024;

    // Determine the type of request and call the appropriate handler
void handleConnection(int socket) {
    char buffer[BUFFER_SIZE] = {0};
    long valread = read(socket, buffer, BUFFER_SIZE);
    if (valread < 0) {
        perror("In read");
        exit(EXIT_FAILURE);
    }
    std::cout << "Received HTTP request: " << std::endl << buffer << std::endl;

    //test to execute the python script (see: https://www.tutorialspoint.com/python/python_cgi_programming.htm)
    // const char* argv[] = { "./cgi-bin/hello_py.cgi", NULL };
    const char* argv[] = { "./cgi-bin/hello.cgi", NULL };
    const char* envp[] = { "QUERY_STRING=Hello from C++ CGI!", NULL };

    std::string response;
    if (strstr(buffer, "GET / HTTP/1.1") || strstr(buffer, "GET /home HTTP/1.1")) {
        response = handleHomePage();
    } else if (strstr(buffer, "GET /hello HTTP/1.1")) {
        response = handleCGIRequest(argv, envp);
    } else {
        response = handleNotFound();
    }

    write(socket, response.c_str(), response.size());
    close(socket);
    }

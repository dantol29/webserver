#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"
#include "include/Environment.hpp"

std::string handleCGIRequest(const char* argv[]) {
    Environment env;
    env.setVar("QUERY_STRING", "Hello from C++ CGI!");

    std::vector<char*> envp = env.getForExecve();

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (!envp.empty()) {
        // Convert the vector to a suitable format for execve
            if (execve(argv[0], const_cast<char* const*>(argv), &envp[0]) == -1) {
                perror("execve failed");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        close(pipefd[1]);

        std::string cgiOutput;
        char readBuffer[256];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], readBuffer, sizeof(readBuffer) - 1)) > 0) {
            readBuffer[bytesRead] = '\0';
            cgiOutput += readBuffer;
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        // Construct the HTTP response
        std::string httpResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n" +
                                   std::string("Content-Length: ") + std::to_string(cgiOutput.length()) + "\n\n" +
                                   cgiOutput;
        
        std::cout << "------------------CGI output prepared-------------------" << std::endl;
        return httpResponse;
    }

    //line necessary to compile
    return "HTTP/1.1 500 Internal Server Error\nContent-Length: 0\n\n";
}

#include "webserv.hpp"
#include "HTTPRequest.hpp"

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

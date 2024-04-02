#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"

const int PORT = 8080;
const int BUFFER_SIZE = 1024;


std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void handleHomePage(int socket) {
    std::string htmlContent = readFile("./home.html");
    std::string httpResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n" +
                               std::string("Content-Length: ") + std::to_string(htmlContent.length()) + "\n\n" +
                               htmlContent;
    write(socket, httpResponse.c_str(), httpResponse.size());
    printf("------------------Home page sent-------------------\n");
}

void handleHelloPage(int socket) {
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

        // non-const char arrays to avoid deprecation warnings
        char scriptPath[] = "./cgi-bin/hello.cgi";
        char *argv[] = {scriptPath, NULL};
        char *envp[] = {"QUERY_STRING=demo hardcoded string query", NULL};
        if (execve(argv[0], argv, envp) == -1) {
            perror("execve failed");
            exit(EXIT_FAILURE);
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

        write(socket, cgiOutput.c_str(), cgiOutput.size());
        printf("------------------CGI output sent-------------------\n");
    }
}


void handleNotFound(int socket) {
    std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    write(socket, response.c_str(), response.size());
}


    // Determine the type of request and call the appropriate handler
void handleConnection(int socket) {
    char buffer[BUFFER_SIZE] = {0};
    long valread = read(socket, buffer, BUFFER_SIZE);
    if (valread < 0) {
        perror("In read");
        exit(EXIT_FAILURE);
    }
    std::cout << "Received HTTP request: " << std::endl << buffer << std::endl;

    if (strstr(buffer, "GET / HTTP/1.1") || strstr(buffer, "GET /home HTTP/1.1")) {
        handleHomePage(socket);
    } else if (strstr(buffer, "GET /hello HTTP/1.1")) {
        handleHelloPage(socket);
    } else {
        handleNotFound(socket);
    }

    close(socket);
}

int main()
{
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
      while (1) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        handleConnection(new_socket);
    }
    return 0;
}
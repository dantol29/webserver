#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <cstdio>       // For popen() and pclose()

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main()
{
    int server_fd, new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE] = {0};

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
    while (1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("In read");
            exit(EXIT_FAILURE);
        }
        std::cout << "Received http request: " << std::endl << buffer << std::endl;

        // Execute the CGI script and get its output
        FILE* pipe = popen("./cgi-bin/hello.cgi", "r");
        if (!pipe) {
            perror("popen failed");
            exit(EXIT_FAILURE);
        }

        // Read the script's output and send it as the response
        std::string cgiOutput;
        char readBuffer[256];
        while (fgets(readBuffer, sizeof(readBuffer), pipe) != NULL) {
            cgiOutput += readBuffer;
        }
        write(new_socket, cgiOutput.c_str(), cgiOutput.size());
        pclose(pipe);

        printf("------------------CGI output sent-------------------\n");

        close(new_socket);
    }
    return 0;
}

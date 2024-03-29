#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <cstdio>       // For popen() and pclose()
#include <fstream>
#include <sstream>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;


/**
 * @brief Reads the entire content of a file into a string.
 *
 * This function opens the file specified by `filePath` and reads its entire content into a single string. It's designed to handle text files. If the file cannot be opened, the function outputs an error message to `std::cerr` and returns an empty string.
 *
 * @param filePath A constant reference to a `std::string` that contains the path to the file to be read.
 * @return A `std::string` containing the contents of the file. Returns an empty string if the file cannot be opened or is otherwise unreadable.
 *
 * @note This function uses `std::ifstream` to open and read the file. It assumes the file's contents are text and can be represented as a string. Binary files or files with data not compatible with `std::string` may not be handled correctly.
 */
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

        // Check if the request is for the root or /home
		if (strstr(buffer, "GET / HTTP/1.1") || strstr(buffer, "GET /home HTTP/1.1")) {
            std::string htmlContent = readFile("./home.html");
            std::string httpResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n" +
                                       std::string("Content-Length: ") + std::to_string(htmlContent.length()) + "\n\n" +
                                       htmlContent;

            write(new_socket, httpResponse.c_str(), httpResponse.size());
            printf("------------------Home page sent-------------------\n");
		}
		 // Check if the request is for /hello
		// The request line for http://localhost:8080/hello 
		// should be a HTTP GET request looks like: GET /hello HTTP/1.1.
        else if (strstr(buffer, "GET /hello HTTP/1.1")) {
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
            write(new_socket, cgiOutput.c_str(), cgiOutput.size()); // Sending the Output to the Client
            pclose(pipe);

            printf("------------------CGI output sent-------------------\n");
        } else {
            // Handle non-/hello requests or send a simple 404 Not Found response
            std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            write(new_socket, response.c_str(), response.size());
        }

        close(new_socket);
  }
    return 0;
}
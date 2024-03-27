#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close

int main(){

    // create socket by using the socket() system call.
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET: It specifies the IPv4 protocol family.
    // SOCK_STREAM: It defines that the TCP type socket.

    // define the server address
    sockaddr_in serverAddress; // data type that is used to store the address of the socket
    serverAddress.sin_family = AF_INET; // specifies the IPv4 protocol family
    serverAddress.sin_port = htons(8080); // convert the unsigned int from machine byte order to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; // we don’t want to bind our socket to any particular IP 
    // and instead make it listen to all the available IPs.

    // bind the socket
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    // bind assigns a specific IP address (often INADDR_ANY for all interfaces) and a port number to the server socket.

    //tell the application to listen to the socket
    listen(serverSocket, 5);
    // 5 indicates that the server's socket can hold a maximum of five pending connection requests before rejecting new ones

    // accept the connection request
    while (1){
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        //block until a request arrives unless configured otherwise

        char buffer[1024];
        // receiving the data from the client
        recv(clientSocket, buffer, sizeof(buffer), 0);
        // Data is received from the client socket and stored in the buffer
        std::cout << "Message from client: " << buffer << std::endl;

    }
    close(serverSocket);
}
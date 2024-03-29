#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen

int main(int argc, char **argv){
    if (argc != 2){
        std::cout << "No more or less than 2 arguments" << std::endl;
        return (1);
    }
     // create socket by using the socket() system call.
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET: It specifies the IPv4 protocol family.
    // SOCK_STREAM: It defines that the TCP type socket.

    // define the server address
    sockaddr_in serverAddress; // data type that is used to store the address of the socket
    serverAddress.sin_family = AF_INET; // specifies the IPv4 protocol family
    serverAddress.sin_port = htons(8080); // convert the unsigned int from machine byte order to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; // we don’t want to bind our socket to any particular IP 
    // and instead make it listen to all the available IPs.

    // establish a connection with the server specified by the serverAddress argument
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // send data to the server
    send(clientSocket, argv[1], strlen(argv[1]), 0);
    close(clientSocket);
}
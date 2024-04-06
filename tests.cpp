#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <sys/wait.h>


// ADD TO THE MAIN IN WEBSERVER (after read())
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;


// ./webserver
// c++ tests.cpp -o test
// ./test

int main(){
	
	const char *requests[] = {
        "GET / HTTP/1.1\r\n", // 200 (OK)
        "POST / HTTP/1.1\r\n", //  200 (OK)
        "GETT / HTTP/1.1\r\n", // 501 (Not Implemented)
        "GET /random HTTP/1.1\r\n", // 400 (Bad Request)
        "GET / HTTP/9.9s\r\n", // 400 (Bad Request)
        " / HTTP/1.1\r\n", // 400 (Bad Request)
		"GET / HTTP/1.1\n", // 400 (Bad Request)
		NULL
    };

	int numRequests = 0;
	while (requests[++numRequests]){}

	int clientSockets[numRequests];
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

	for (int i = 0; i < numRequests; i++){
		clientSockets[i] = socket(AF_INET, SOCK_STREAM, 0);
    	connect(clientSockets[i], (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    	send(clientSockets[i], requests[i], strlen(requests[i]), 0);
    	close(clientSockets[i]);
		sleep(2);
	}
}
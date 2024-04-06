#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <iostream>
#include <unistd.h> // close
#include <string.h> // strlen
#include <sys/wait.h>
#include <cstring>

// ADD TO THE MAIN IN WEBSERVER (after read())
// #include "HTTPRequest.hpp"
// HTTPRequest obj(buffer);
// std::cout << obj.getStatusCode() << std::endl;


// ./webserver
// c++ tests.cpp -o test
// ./test "test_name"

void	sendData(const char *requests[], sockaddr_in serverAddress)
{
	int numRequests = 0;
	while (requests[++numRequests]){}

	int clientSockets[numRequests];
    

	for (int i = 0; i < numRequests; i++){
		clientSockets[i] = socket(AF_INET, SOCK_STREAM, 0);
    	connect(clientSockets[i], (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    	send(clientSockets[i], requests[i], strlen(requests[i]), 0);
    	close(clientSockets[i]);
		sleep(2);
	}
}

void	simple(sockaddr_in serverAddress)
{
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
	sendData(requests, serverAddress);
}

void	query(sockaddr_in serverAddress)
{
	const char *requests[] = {
        "GET /search?q=now&price=low HTTP/1.1\r\n", // 200 (OK)
        "POST /search?q==now&price=low HTTP/1.1\r\n", //  400 (Bad Request)
        "GET /search??q=now&price=low HTTP/1.1\r\n", // 400 (Bad Request)
        "GET /search?now&price=low HTTP/1.1\r\n", // 400 (Bad Request)
        "GET /search?q=now&&price=low HTTP/1.1\r\n", // 400 (Bad Request)
		"GET /search?q=now&price=low= HTTP/1.1\r\n", // 400 (Bad Request)
		"GET /search?=now&price=low HTTP/1.1\r\n", // 400 (Bad Request)
		"GET /search?&q=now&price=low HTTP/1.1\r\n", // 400 (Bad Request)
		NULL
    };
	sendData(requests, serverAddress);
}

int main(int argc, char **argv){
	
	if (argc != 2)
		return (1);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (std::strcmp(argv[1], "query") == 0)
		query(serverAddress);
	else if (std::strcmp(argv[1], "simple") == 0)
		simple(serverAddress);
	else
		std::cout << "Invalid test name" << std::endl;
	
}
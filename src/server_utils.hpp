#ifndef SERVER_UTILS_HPP
#define SERVER_UTILS_HPP

#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>		// For read, write, and close
#include <fstream>
#include <sstream>
#include "webserv.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Connection.hpp"
#include "MetaVariables.hpp"
#include "Router.hpp"
#include <poll.h>

class Connection; // Forward declaration for circular dependency

const int BUFFER_SIZE = 1024;
const size_t CLIENT_MAX_HEADERS_SIZE = 8192; // 8KB - This is the limit of the header size also in NGINX
const size_t CLIENT_MAX_BODY_SIZE = 1048576; // 1MB - This is the limit of the body size also in NGINX

std::string handleCGIRequest(const char *argv[], MetaVariables env);

// bool isChunked(const std::string &headers);
size_t getContentLength(const std::string &headers);
// bool readChunkSize(int socket, std::string &line);
// bool readChunk(int socket, size_t chunkSize, std::string &chunkedData, HTTPResponse &response);
void printVariablesHeadersBody(const HTTPRequest &obj);
void perrorAndExit(const char *msg);
char customToLower(char c);
void printFDsVector(const std::vector<pollfd> &fds);
void print_connectionsVector(const std::vector<Connection> &connections);

#endif // SERVER_UTILS_HPP
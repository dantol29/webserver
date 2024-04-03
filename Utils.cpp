#include <cstdlib> // For exit() and EXIT_FAILURE
#include <cstring> // For memset
#include <iostream>
#include <netinet/in.h> // For sockaddr_in
#include <sys/socket.h> // For socket functions
#include <unistd.h>     // For read, write, and close
#include <fstream>
#include <sstream>
#include "include/webserv.hpp"

std::string readHtml(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void *ft_memset(void *ptr, int value, size_t num)
{
	// Cast the pointer to a char pointer, as we're dealing with bytes
	unsigned char *p = static_cast<unsigned char *>(ptr);

	// Fill the specified memory area with the given value
	for (size_t i = 0; i < num; ++i)
	{
		p[i] = static_cast<unsigned char>(value);
	}

	// Return the original pointer
	return ptr;
}
#include <iostream>
#include <string>
#include <cctype> // For std::isspace

void printHTTPRequest(const std::string &httpRequest, size_t position) {
    std::cout << "HTTP Request at position " << position << ": " << httpRequest.substr(position, 50) << "..." << std::endl;
}

void printHexThird(const std::string &str, size_t startPos, size_t length) {
    std::cout << "Printing hex (mocked) for third of string from position " << startPos << " for length " << length << std::endl;
}

void printHex(const std::string &str) {
    std::cout << "Printing hex (mocked) for string: " << str << std::endl;
}


int extractContentLengthAlt(const std::string &httpRequest)
{
    std::cout << "Extracting Content-Length from HTTP header ..." << std::endl;
    size_t startPos = httpRequest.find("Content-Length:");
    if (startPos == std::string::npos)
    {
        std::cout << "Content-Length not found in the request." << std::endl;
        return -1;
    }
    std::cout << "startPos: " << startPos << std::endl;
    printHTTPRequest(httpRequest, startPos);
    startPos += 15; // Move past "Content-Length:"
    // std::cout << "startPos after moving: " << startPos << std::endl;
    printHTTPRequest(httpRequest, startPos);
    while (startPos < httpRequest.size() && std::isspace(httpRequest[startPos]))
        startPos++; // Skip any spaces after the colon
    // std::cout << "startPos after skipping spaces: " << startPos << std::endl;
    size_t endPos = httpRequest.find("\r\n", startPos);
    std::cout << "endPos: " << endPos << std::endl;
    printHTTPRequest(httpRequest, endPos);
    if (endPos == std::string::npos)
    {
        std::cout << "Malformed HTTP header, no CRLF after Content-Length." << std::endl;
        printHTTPRequest(httpRequest, startPos);
        // printHexStartLength(httpRequest, startPos, 50);
        printHexThird(httpRequest, startPos, 50);
        return -1;
    }

    std::string contentLengthStr = httpRequest.substr(startPos, endPos - startPos);
    std::cout << "Substring for Content-Length: '" << contentLengthStr << "'" << std::endl;
    printHex(contentLengthStr); // Print the substring in hex to check for hidden characters

    try
    {
        int contentLength = std::stoi(contentLengthStr);
        std::cout << "Parsed Content-Length: " << contentLength << std::endl;
        return contentLength;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to parse Content-Length: " << e.what() << std::endl;
        return -1;
    }
}

int main() {
    // Example HTTP requests for testing
    std::string httpRequest1 = "GET / HTTP/1.1\r\nHost: example.com\r\nContent-Length: 12345\r\n\r\n";
    std::string httpRequest2 = "POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/plain\r\nContent-Length: 678\r\n\r\n";
    std::string httpRequest3 = "GET / HTTP/1.1\r\nHost: example.com\r\n"; // Missing Content-Length

    std::cout << "Test 1:\n";
    int length1 = extractContentLengthAlt(httpRequest1);
    std::cout << "Extracted Content-Length: " << length1 << "\n\n";

    std::cout << "Test 2:\n";
    int length2 = extractContentLengthAlt(httpRequest2);
    std::cout << "Extracted Content-Length: " << length2 << "\n\n";

    std::cout << "Test 3:\n";
    int length3 = extractContentLengthAlt(httpRequest3);
    std::cout << "Extracted Content-Length: " << length3 << "\n\n";

    return 0;
}

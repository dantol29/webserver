#include "webserv.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <sys/wait.h>
#include <string>
#include <map>

void RequestTargetToMetaVars(HTTPRequest request, Environment& env) {
    std::string requestTarget = request.getRequestTarget();

    if (requestTarget.empty()) {
        std::cout << "Request target is empty" << std::endl;
        return;
    } else if (requestTarget[0] == '/') {
        std::cout << "Identified Origin-Form request target" << std::endl;
        env.setVar("SCRIPT_NAME", "--------⚠️---WAITING FOR DANIIL's IMPLEMENTATION--⚠️---------");
        env.setVar("QUERY_STRING", "--------⚠️--WAITING FOR DANIIL's IMPLEMENTATION--⚠️---------");
        std::cout << "QUERY_STRING set to : " << env.getVar("QUERY_STRING") << std::endl; // Assuming getVar method exists
        std::cout << "SCRIPT_NAME set to : " << env.getVar("SCRIPT_NAME") << std::endl; // Assuming getVar method exists
    } else if (startsWith(requestTarget, "http")) {
        std::cout << "Identified Absolute-Form request target" << std::endl;
        // No direct action for CGI variables
    } else if (requestTarget.find("/") == std::string::npos && requestTarget != "*" && !startsWith(requestTarget, "http")) {
        std::cout << "Identified Authority-Form request target" << std::endl;
        // No direct action for CGI variables
    } else if (requestTarget == "*") {
        std::cout << "Identified Asterisk-Form request target: " << requestTarget << std::endl;
        env.setVar("REQUEST_METHOD", "OPTIONS");
        std::cout << "REQUEST_METHOD set to OPTIONS" << std::endl;
    } else {
        std::cout << "Unrecognized Request Target Form: " << requestTarget << std::endl;
    }

    // SHOULD I ALWAYS set GATEWAY_INTERFACE ?
    env.setVar("GATEWAY_INTERFACE", "CGI/1.1");
}


//refer to RFC 3875 for more information on CGI environment variables
void HTTPRequestToMetaVars(char* rawRequest, Environment& env) {
    HTTPRequest request(rawRequest);

    // Set the method used for the request (e.g., GET, POST)
    env.setVar("REQUEST_METHOD", request.getMethod());
    // Set the protocol version used in the request (e.g., HTTP/1.1)
    env.setVar("PROTOCOL_VERSION", request.getProtocolVersion());

    // Server-related variables
    // The name and version of the HTTP server (Format: name/version)
    env.setVar("SERVER_SOFTWARE", "Server_of_people_identifying_as_objects/1.0");
    // The host name, DNS alias, or IP address of the server
    env.setVar("SERVER_NAME", "The_objects.com");
    // The CGI specification revision the server is using (Format: CGI/version)
    env.setVar("GATEWAY_INTERFACE", "CGI/1.1");

    // Request-specific variables
    // The name and revision of the protocol the request was made in (Format: protocol/revision)
    env.setVar("SERVER_PROTOCOL", request.getProtocolVersion());
    // The port number on which the request was received
    env.setVar("SERVER_PORT", "8080"); //     ---> how to set it programmatically ? from the macro ?
    // Additional path information from the client's request URL
    env.setVar("PATH_INFO", ""); 
    // The translated physical path the request refers to (after virtual to physical conversion by the server)
    env.setVar("PATH_TRANSLATED", ""); // Needs specific server-side logic to determine
    // The virtual path to the script being executed
    env.setVar("SCRIPT_NAME", "");
    // The query string from the URL sent by the client
    env.setVar("QUERY_STRING", "request.getQueryString()");

    //The REMOTE_HOST variable contains the fully qualified domain name of
   //the client sending the request to the server
    env.setVar("REMOTE_HOST", ""); // Might require reverse DNS lookup
    // network address (IP) of the client sending the request to the server.
    env.setVar("REMOTE_ADDR", ""); // Needs to be obtained from the request/connection

    //AUTHENTICATION :
    // The authentication method used to protect the script
    env.setVar("AUTH_TYPE", ""); // Depends on server configuration
    // The client's username, if the script is protected and the server supports authentication
    env.setVar("REMOTE_USER", ""); // Depends on server and authentication method
    // The remote (client's) username from RFC 931 identification; for log purposes only
    env.setVar("REMOTE_IDENT", ""); // Requires specific server support

    //ONLY FOR POST REQUESTS ?    OR NOT ?
    // The content type attached to the request, if any
    env.setVar("CONTENT_TYPE", ""); // Needs to be parsed from the request headers
    // The length of the content sent by the client
    env.setVar("CONTENT_LENGTH", ""); // Needs to be parsed from the request headers
}

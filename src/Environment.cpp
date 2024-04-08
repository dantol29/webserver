#include "include/webserv.hpp"
#include <string.h>
#include <utility>

Environment::Environment() {
}

Environment::Environment(const Environment& other) : envVars(other.envVars) {
}

Environment& Environment::operator=(const Environment& other) {
    if (this != &other) { // Protect against self-assignment
        envVars = other.envVars; // Use std::map's assignment operator for a deep copy
    }
    return *this;
}

/**
 * @brief Sets or updates an environment variable.
 * 
 * This method allows setting a new environment variable or updating the value
 * of an existing one in the internal environment variable storage.
 * 
 * @param key The name of the environment variable.
 * @param value The value to be assigned to the environment variable.
 */
void Environment::setVar(const std::string& key, const std::string& value) {
	envVars[key] = value;
}

//it does not modify any member variables of the Environment class
// (hence the const at the end of the function signature).
std::string Environment::getVar(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = envVars.find(key);
    if (it != envVars.end()) {
        return it->second;
    } else {
        return "";
    }
}

/**
 * @brief Generates a vector of C-style strings suitable for execve.
 * 
 * This method converts the stored environment variables into a format that can
 * be used with the execve system call. Each string in the returned vector is
 * in the form of "key=value". The memory for these strings is dynamically
 * allocated and must be freed by the caller. The list is terminated with a NULL
 * pointer as required by execve.
 * 
 * @return A vector of C-style strings representing the environment variables,
 *         suitable for passing to execve.
 */
std::vector<char*> Environment::getForExecve() const {
	std::vector<char*> result;
	for (std::map<std::string, std::string>::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
		std::string env = it->first + "=" + it->second;
		char* envCStr = new char[env.size() + 1];
		ft_strcpy(envCStr, env.c_str());
		result.push_back(envCStr);
	}
	result.push_back(NULL);
	return result;
}

// because at the moment we implement only GET, POST and DELETE methods
// we don't need that, but we can add it later
bool Environment::isAuthorityForm(const HTTPRequest& request) {
    std::string method = request.getMethod();
    std::string requestTarget = request.getRequestTarget();

    if (method != "CONNECT") {
        return false;
    }

    if (requestTarget.find("://") != std::string::npos || requestTarget[0] == '/') {
        return false;
    }
    return true;
}

/**
 * Separates the request target into the script path and path info.
 *
 * @param requestTarget The full request target URL from the client.
 * @return A pair where the first element is the script virtual path and the
 *         second element is the additional path info.
 */
std::pair<std::string, std::string> separatePathAndInfo(const std::string& requestTarget) {
    std::string::size_type scriptEndPos = requestTarget.find('/', 1); 
    if (scriptEndPos == std::string::npos) {
        // script path only, no PATH_INFO
        return std::pair<std::string, std::string>(requestTarget, "");
    } else {
        std::string scriptPath = requestTarget.substr(0, scriptEndPos);
        std::string pathInfo = requestTarget.substr(scriptEndPos);

        return std::make_pair(scriptPath, pathInfo);
    }
}


void  Environment::RequestTargetToMetaVars(HTTPRequest request, Environment& env) {
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
    } else if (requestTarget == "*") {
        std::cout << "Identified Asterisk-Form request target: " << requestTarget << std::endl;
        env.setVar("REQUEST_METHOD", "OPTIONS");
        std::cout << "REQUEST_METHOD set to OPTIONS" << std::endl;
    } else if (isAuthorityForm(request)) {
        std::cout << "Identified Authority-Form request target" << std::endl;
        // No direct action for CGI variables
    } else {
        std::cout << "Unrecognized Request Target Form: " << requestTarget << std::endl;
    }

    // SHOULD I ALWAYS set GATEWAY_INTERFACE ?
    env.setVar("GATEWAY_INTERFACE", "CGI/1.1");
}


//refer to RFC 3875 for more information on CGI environment variables
void  Environment::HTTPRequestToMetaVars(char* rawRequest, Environment& env) {
    HTTPRequest request(rawRequest);


    //________General variables
    // Set the method used for the request (e.g., GET, POST)
    env.setVar("REQUEST_METHOD", request.getMethod());
    // Set the protocol version used in the request (e.g., HTTP/1.1)
    env.setVar("PROTOCOL_VERSION", request.getProtocolVersion());
    env.setVar("SERVER_PORT", "8080"); //     ---> how to set it programmatically ? from the macro ?


    //_______Server-related variables
    // The name and version of the HTTP server (Format: name/version)
    env.setVar("SERVER_SOFTWARE", "Server_of_people_identifying_as_objects/1.0");
    // The host name, DNS alias, or IP address of the server
    env.setVar("SERVER_NAME", "The_objects.com");
    // The CGI specification revision the server is using (Format: CGI/version)
    env.setVar("GATEWAY_INTERFACE", "CGI/1.1");





    //_______Request-related variables
    std::pair<std::string, std::string> pathComponents = separatePathAndInfo(request.getRequestTarget());

    std::string scriptVirtualPath = pathComponents.first; // path to the script
    std::string pathInfo = pathComponents.second; // path after the script
    env.setVar("PATH_INFO", pathInfo);
    // most likely append the PATH_INFO to the root directory of the script OR MAYBE use a specific mapping logic
    // std::string pathTranslated = translatePathToPhysical(scriptVirtualPath, pathInfo); // Implement this function 
    // env.setVar("PATH_TRANSLATED", pathTranslated);

    // The virtual path to the script being executed
    env.setVar("SCRIPT_NAME", "");
    // The query string from the URL sent by the client
    env.setVar("QUERY_STRING", "request.getQueryString()");


    //The REMOTE_HOST variable contains the fully qualified domain name of
   //the client sending the request to the server
    env.setVar("REMOTE_HOST", ""); // Might require reverse DNS lookup
    // network address (IP) of the client sending the request to the server.
    env.setVar("REMOTE_ADDR", ""); // Needs to be obtained from the request/connection


    //_______AUTHENTICATION :
    // The authentication method used to protect the script
    env.setVar("AUTH_TYPE", ""); // Depends on server configuration
    // The client's username, if the script is protected and the server supports authentication
    env.setVar("REMOTE_USER", ""); // Depends on server and authentication method
    // The remote (client's) username from RFC 931 identification; for log purposes only
    env.setVar("REMOTE_IDENT", ""); // Requires specific server support


    //_______
    //ONLY FOR POST REQUESTS ?    OR NOT ?
    // The content type attached to the request, if any
    env.setVar("CONTENT_TYPE", ""); // Needs to be parsed from the request headers



    //_______set the metadata from the headers of the request
    std::pair<std::string, std::string> contentTypeHeader = request.getHeaders("Content-Type");
    if (!contentTypeHeader.first.empty()) {
        env.setVar("CONTENT_TYPE", contentTypeHeader.second);
    } else {
        // set CONTENT_TYPE to an empty string or a default value.
        env.setVar("CONTENT_TYPE", "");
    }
    std::pair<std::string, std::string> contentLengthHeader = request.getHeaders("Content-Length");
    if (!contentLengthHeader.first.empty()) {
        env.setVar("CONTENT_LENGTH", contentLengthHeader.second);
    } else {
        // set CONTENT_TYPE to 0 if not present
        env.setVar("CONTENT_LENGTH", "0");
    }
}


Environment::~Environment() {
	std::vector<char*> envp = getForExecve();
	for (size_t i = 0; i < envp.size(); ++i) {
		delete[] envp[i];
	}
}

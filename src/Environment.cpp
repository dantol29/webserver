#include "webserv.hpp"
#include <string.h>
#include <utility>

Environment::Environment()
{
}

Environment::Environment(const Environment &other) : metaVars(other.metaVars)
{
}

Environment &Environment::operator=(const Environment &other)
{
	if (this != &other)
	{							 // Protect against self-assignment
		metaVars = other.metaVars; // Use std::map's assignment operator for a deep copy
	}
	return *this;
}

/**
 * @brief Sets or updates an environment variable.
 */
void Environment::setVar(const std::string &key, const std::string &value)
{
	metaVars[key] = value;
}

// it does not modify any member variables of the Environment class
//  (hence the const at the end of the function signature).
std::string Environment::getVar(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = metaVars.find(key);
	if (it != metaVars.end())
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

void Environment::printMetaVars() const {
    for (std::map<std::string, std::string>::const_iterator it = metaVars.begin(); it != metaVars.end(); ++it) {
        std::cout << it->first << " = " << it->second << std::endl;
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
std::vector<char *> Environment::getForExecve() const
{
	std::vector<char *> result;
	for (std::map<std::string, std::string>::const_iterator it = metaVars.begin(); it != metaVars.end(); ++it)
	{
		std::string env = it->first + "=" + it->second;
		char *envCStr = new char[env.size() + 1];
		size_t i;
		for (i = 0; i < env.size(); ++i)
		{
			envCStr[i] = env[i];
		}
		envCStr[i] = '\0'; // Null-terminate the string
		result.push_back(envCStr);
	}
	result.push_back(NULL);
	return result;
}

/**
 * @brief Divide request target into SCRIPT_NAME and PATH_INFO.
 *
 * note: SCRIPT_NAME = URI path to identify CGI script, not just the name of the script
 *
 * Searches the request target for known script extensions
 * (.cgi, .pl, .py, .php)
 * Identifies the script extension
 * splits the request target : before is the script path, after is path info.
 *
 * @param requestTarget The full request target URL from the client.
 * @return std::pair<std::string, std::string> A pair where the first element
 *         is the script path (up to and including the script extension) and
 *         the second element is the additional path info (anything after the script path).
 */
std::pair<std::string, std::string> Environment::separatePathAndInfo(const std::string &requestTarget) const
{
	const char *extensions[] = {".cgi", ".pl", ".py", ".php"};
	const size_t extCount = sizeof(extensions) / sizeof(extensions[0]);

	std::string scriptPath;
	std::string pathInfo;

	size_t pos = std::string::npos; // largest possible value for size_t =  initialization to "not found"
	for (size_t i = 0; i < extCount; ++i)
	{
		pos = requestTarget.find(extensions[i]);
		if (pos != std::string::npos)
		{
			pos += strlen(extensions[i]); // add extension length
			break;
		}
	}

	if (pos != std::string::npos)
	{
		// Debugging print line
		std::cout << "Script extension found. Script Path: '" << scriptPath << "', Path Info: '" << pathInfo << "'"
				  << std::endl;
		// Everything before the position is considered the script path
		scriptPath = requestTarget.substr(0, pos);
		// Everything after the script is considered path info
		if (pos < requestTarget.length())
			pathInfo = requestTarget.substr(pos);
	}
	else
	{
		// No extension is found,return entire target as script path
		scriptPath = requestTarget;
	}
	// Debugging print before return
	std::cout << "Returning - Script Path: '" << scriptPath << "', Path Info: '" << pathInfo << "'" << std::endl;

	return std::make_pair(scriptPath, pathInfo);
}

// Utility function to check if 'str' starts with the given 'prefix'
// Conforms to C++98 standard
bool startsWith(const std::string &str, const std::string &prefix)
{
	if (str.length() < prefix.length())
	{
		return false; // str is shorter than prefix, so it cannot start with prefix
	}
	// Check each character up to the length of 'prefix' to ensure they match
	for (std::string::size_type i = 0; i < prefix.length(); ++i)
	{
		if (str[i] != prefix[i])
		{
			return false; // Found a mismatch
		}
	}
	return true; // All characters matched
}

// because at the moment we implement only GET, POST and DELETE methods
// we don't need that, but we can add it later
bool Environment::isAuthorityForm(const HTTPRequest &request)
{
	std::string method = request.getMethod();
	std::string requestTarget = request.getRequestTarget();

	if (method != "CONNECT")
	{
		return false;
	}

	if (requestTarget.find("://") != std::string::npos || requestTarget[0] == '/')
	{
		return false;
	}
	return true;
}

// This function is used to set the CGI environment variables based on the request target.
//  atm this is more of a debug function, especially as we implement only GET, POST and DELETE
// or a draft for a future more advanced implementation
void Environment::RequestTargetToMetaVars(HTTPRequest request, Environment &env)
{
	std::string requestTarget = request.getRequestTarget();

	if (requestTarget.empty())
	{
		std::cout << "Request target is empty" << std::endl;
		return;
	}
	else if (requestTarget[0] == '/')
	{
		std::cout << "Identified Origin-Form request target" << std::endl;
		//
	}
	else if (startsWith(requestTarget, "http"))
	{
		std::cout << "Identified Absolute-Form request target" << std::endl;
		// No direct action for CGI variables
	}
	else if (requestTarget == "*")
	{
		std::cout << "Identified Asterisk-Form request target: " << requestTarget << std::endl;
		env.setVar("REQUEST_METHOD", "OPTIONS");
		std::cout << "REQUEST_METHOD set to OPTIONS" << std::endl;
	}
	else if (isAuthorityForm(request))
	{
		std::cout << "Identified Authority-Form request target" << std::endl;
		// No direct action for CGI variables
	}
	else
	{
		std::cout << "Unrecognized Request Target Form: " << requestTarget << std::endl;
	}

	// SHOULD I ALWAYS set GATEWAY_INTERFACE ?
	env.setVar("GATEWAY_INTERFACE", "CGI/1.1");
}

/**
 * @brief Constructs a query string from query parameters as multimap.
 *
 * Takes a multimap of query parameters
 * concatenates them into a single string.
 * - Each key-value pair is joined by '=',
 * - pairs are separated by '&'.
 * The resulting string is formatted for use in a URL query string.
 *
 * @param queryParams The multimap containing query parameters.
 * a reference to a constant std::multimap (that maps std::string keys to std::string values.)
 * @return A string representing the formatted query string.
 */
std::string Environment::formatQueryString(const std::multimap<std::string, std::string> &queryParams) const
{
	std::string queryString;
	for (std::multimap<std::string, std::string>::const_iterator it = queryParams.begin(); it != queryParams.end();)
	{
		queryString += it->first + "=" + it->second;
		++it;
		if (it != queryParams.end())
		{
			queryString += "&";
		}
	}
	return queryString;
}
void Environment::subtractQueryFromPathInfo(std::string& pathInfo, const std::string& queryString) {
	if (queryString.empty()) {
		return;
	}

	if (pathInfo.length() >= queryString.length() &&
		pathInfo.compare(pathInfo.length() - queryString.length(), queryString.length(), queryString) == 0) {
		pathInfo.erase(pathInfo.length() - queryString.length());
	}

}


// RFC 3875 for more information on CGI environment variables, or README_CGI_ENV.md
void Environment::HTTPRequestToMetaVars(HTTPRequest request, Environment &env)
{
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

	//_______Path-related variables
	std::string queryString = formatQueryString(request.getQueryString());
	env.setVar("QUERY_STRING", queryString);
	std::pair<std::string, std::string> pathComponents = separatePathAndInfo(request.getRequestTarget());
	std::string scriptName = pathComponents.first; // path to the script
	std::string pathInfo = pathComponents.second;  // path after the script

	subtractQueryFromPathInfo(pathInfo, queryString);

	env.setVar("PATH_INFO", pathInfo);
	// most likely append the PATH_INFO to the root directory of the script OR MAYBE use a specific mapping logic
	// std::string pathTranslated = translatePathToPhysical(scriptVirtualPath, pathInfo); // Implement this function
	env.setVar("PATH_TRANSLATED", scriptName);//TEMPORARY
	// SCRIPT_NAME = URI path to identify CGI script, not just the name of the script
	env.setVar("SCRIPT_NAME", scriptName);
	// The query string from the URL sent by the client

	// The REMOTE_HOST variable contains the fully qualified domain name of
	// the client sending the request to the server
	//  env.setVar("REMOTE_HOST", ""); // Might require reverse DNS lookup
	//  // network address (IP) of the client sending the request to the server.
	//  env.setVar("REMOTE_ADDR", ""); // Needs to be obtained from the request/connection

	//_______AUTHENTICATION :
	// // The authentication method used to protect the script
	// env.setVar("AUTH_TYPE", ""); // Depends on server configuration
	// // The client's username, if the script is protected and the server supports authentication
	// env.setVar("REMOTE_USER", ""); // Depends on server and authentication method
	// // The remote (client's) username from RFC 931 identification; for log purposes only
	// env.setVar("REMOTE_IDENT", ""); // Requires specific server support

	//_______set the metadata from the headers of the request
	std::pair<std::string, std::string> contentTypeHeader = request.getHeaders("Content-Type");
	if (!contentTypeHeader.first.empty())
	{
		env.setVar("CONTENT_TYPE", contentTypeHeader.second);
	}
	else
	{
		// set CONTENT_TYPE to an empty string            --> or a default value ?
		env.setVar("CONTENT_TYPE", "");
	}
	std::pair<std::string, std::string> contentLengthHeader = request.getHeaders("Content-Length");
	if (!contentLengthHeader.first.empty())
	{
		env.setVar("CONTENT_LENGTH", contentLengthHeader.second);
	}
	else
	{
		// set CONTENT_TYPE to 0 if not present
		env.setVar("CONTENT_LENGTH", "0");
	}
}

Environment::~Environment()
{
	std::vector<char *> envp = getForExecve();
	for (size_t i = 0; i < envp.size(); ++i)
	{
		delete[] envp[i];
	}
}

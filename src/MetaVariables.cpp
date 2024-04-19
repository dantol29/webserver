#include <cstring>
#include <utility>
#include <MetaVariables.hpp>

MetaVariables::MetaVariables()
{
}

MetaVariables::MetaVariables(const MetaVariables &other) : metaVars(other.metaVars)
{
}

MetaVariables &MetaVariables::operator=(const MetaVariables &other)
{
	if (this != &other)
	{
		metaVars = other.metaVars;
	}
	return *this;
}

void MetaVariables::setVar(const std::string &key, const std::string &value)
{
	metaVars[key] = value;
}

std::string MetaVariables::getVar(const std::string &key) const
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

std::vector<char *> MetaVariables::getForExecve() const
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
		envCStr[i] = '\0';
		result.push_back(envCStr);
	}
	result.push_back(NULL);
	return result;
}

std::pair<std::string, std::string> MetaVariables::separatePathAndInfo(const std::string &requestTarget) const
{
	const char *extensions[] = {".cgi", ".pl", ".py", ".php"};
	const size_t extCount = sizeof(extensions) / sizeof(extensions[0]);

	std::string scriptPath;
	std::string pathInfo;

	size_t pos = std::string::npos;
	for (size_t i = 0; i < extCount; ++i)
	{
		pos = requestTarget.find(extensions[i]);
		if (pos != std::string::npos)
		{
			pos += strlen(extensions[i]);
			break;
		}
	}

	if (pos != std::string::npos)
	{
		scriptPath = requestTarget.substr(0, pos);
		if (pos < requestTarget.length())
			pathInfo = requestTarget.substr(pos);
	}
	else
	{
		scriptPath = requestTarget;
	}

	return std::make_pair(scriptPath, pathInfo);
}

bool startsWith(const std::string &str, const std::string &prefix)
{
	if (str.length() < prefix.length())
	{
		return false;
	}
	for (std::string::size_type i = 0; i < prefix.length(); ++i)
	{
		if (str[i] != prefix[i])
		{
			return false;
		}
	}
	return true;
}

// bool MetaVariables::isAuthorityForm(const HTTPRequest &request)
// {
// 	std::string method = request.getMethod();
// 	std::string requestTarget = request.getRequestTarget();

// 	if (method != "CONNECT")
// 	{
// 		return false;
// 	}

// 	if (requestTarget.find("://") != std::string::npos || requestTarget[0] == '/')
// 	{
// 		return false;
// 	}
// 	return true;
// }

// void MetaVariables::RequestTargetToMetaVars(HTTPRequest request, MetaVariables &env)
// {
// 	std::string requestTarget = request.getRequestTarget();

// 	if (requestTarget.empty())
// 	{
// 		std::cout << "Request target is empty" << std::endl;
// 		return;
// 	}
// 	else if (requestTarget[0] == '/')
// 	{
// 		std::cout << "Identified Origin-Form request target" << std::endl;
// 		//
// 	}
// 	else if (startsWith(requestTarget, "http"))
// 	{
// 		std::cout << "Identified Absolute-Form request target" << std::endl;
// 		// No direct action for CGI variables
// 	}
// 	else if (requestTarget == "*")
// 	{
// 		std::cout << "Identified Asterisk-Form request target: " << requestTarget << std::endl;
// 		env.setVar("REQUEST_METHOD", "OPTIONS");
// 		std::cout << "REQUEST_METHOD set to OPTIONS" << std::endl;
// 	}
// 	else if (isAuthorityForm(request))
// 	{
// 		std::cout << "Identified Authority-Form request target" << std::endl;
// 		// No direct action for CGI variables
// 	}
// 	else
// 	{
// 		std::cout << "Unrecognized Request Target Form: " << requestTarget << std::endl;
// 	}

// 	env.setVar("GATEWAY_INTERFACE", "CGI/1.1");
// }

std::string MetaVariables::formatQueryString(const std::multimap<std::string, std::string> &queryParams) const
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
void MetaVariables::subtractQueryFromPathInfo(std::string &pathInfo, const std::string &queryString)
{
	if (queryString.empty())
	{
		return;
	}

	if (pathInfo.length() >= queryString.length() &&
		pathInfo.compare(pathInfo.length() - queryString.length(), queryString.length(), queryString) == 0)
	{
		pathInfo.erase(pathInfo.length() - queryString.length());
	}
}

void MetaVariables::HTTPRequestToMetaVars(const HTTPRequest &request, MetaVariables &env)
{
	// env.setVar("X_INTERPRETER_PATH", "/home/lmangall/.brew/bin/python3"); // school computer...

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
	std::string scriptName = pathComponents.first;
	std::string pathInfo = pathComponents.second;

	subtractQueryFromPathInfo(pathInfo, queryString);

	env.setVar("PATH_INFO", pathInfo);
	// most likely append the PATH_INFO to the root directory of the script OR MAYBE use a specific mapping logic
	// std::string pathTranslated = translatePathToPhysical(scriptVirtualPath, pathInfo); // Implement this function
	env.setVar("PATH_TRANSLATED", "var/www"); // TEMPORARY
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
	std::pair<std::string, std::string> contentTypeHeader = request.getSingleHeader("Content-Type");
	if (!contentTypeHeader.first.empty())
	{
		env.setVar("CONTENT_TYPE", contentTypeHeader.second);
	}
	else
	{
		env.setVar("CONTENT_TYPE", "");
	}
	std::pair<std::string, std::string> contentLengthHeader = request.getSingleHeader("Content-Length");
	if (!contentLengthHeader.first.empty())
	{
		env.setVar("CONTENT_LENGTH", contentLengthHeader.second);
	}
	else
	{
		env.setVar("CONTENT_LENGTH", "0");
	}
}

MetaVariables::~MetaVariables()
{
	std::vector<char *> envp = getForExecve();
	for (size_t i = 0; i < envp.size(); ++i)
	{
		delete[] envp[i];
	}
}

std::ostream &operator<<(std::ostream &out, const MetaVariables &instancePrinted)
{
	out << "\033[35m";
	out << "MetaVariables Variables:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = instancePrinted.metaVars.begin();
		 it != instancePrinted.metaVars.end();
		 ++it)
	{
		out << it->first << "=" << it->second << std::endl;
	}

	out << "\033[0m";
	return out;
}

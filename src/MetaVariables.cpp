#include <cstring>
#include <utility>
#include <MetaVariables.hpp>

#define X_INTERPRETER_PATH ""
// X_INTERPRETER_PATH "/home/lmangall/.brew/bin/python3";

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

std::vector<std::string> MetaVariables::getForExecve() const
{
	std::vector<std::string> result;
	for (std::map<std::string, std::string>::const_iterator it = metaVars.begin(); it != metaVars.end(); ++it)
	{
		std::string env = it->first + "=" + it->second;
		result.push_back(env);
	}
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
	env.setVar("X_INTERPRETER_PATH", X_INTERPRETER_PATH);
	env.setVar("REQUEST_METHOD", request.getMethod());
	env.setVar("PROTOCOL_VERSION", request.getProtocolVersion());
	env.setVar("SERVER_PORT", request.getSingleHeader("listen").second);
	env.setVar("SERVER_SOFTWARE", "Server_of_people_identifying_as_objects/1.0");
	env.setVar("SERVER_NAME", request.getSingleHeader("host").second);
	env.setVar("GATEWAY_INTERFACE", "CGI/1.1");
	std::string queryString = formatQueryString(request.getQueryString());
	env.setVar("QUERY_STRING", queryString);

	Debug::log("MetaVariables::HTTPRequestToMetaVars:  request.getRequestTarget(): " + request.getRequestTarget(),
			   Debug::CGI);

	std::pair<std::string, std::string> pathComponents = separatePathAndInfo(request.getRequestTarget());
	Debug::log("MetaVariables::HTTPRequestToMetaVars:  pathComponents.second: " + pathComponents.second, Debug::CGI);

	std::string root = request.getRoot();
	std::string host = request.getSingleHeader("host").second;
	std::string scriptName = pathComponents.first;
	std::string pathInfo = pathComponents.second;
	Debug::log("MetaVariables::HTTPRequestToMetaVars: root: " + root, Debug::NORMAL);
	Debug::log("MetaVariables::HTTPRequestToMetaVars: host: " + host, Debug::NORMAL);
	Debug::log("MetaVariables::HTTPRequestToMetaVars: scriptName: " + scriptName, Debug::NORMAL);
	Debug::log("MetaVariables::HTTPRequestToMetaVars: pathInfo: " + pathInfo, Debug::NORMAL);
	subtractQueryFromPathInfo(pathInfo, queryString);
	env.setVar("PATH_INFO", pathInfo);
	env.setVar("PATH_TRANSLATED", root + host + scriptName);
	env.setVar("SCRIPT_NAME", scriptName);

	std::pair<std::string, std::string> contentTypeHeader = request.getSingleHeader("Content-Type");
	if (!contentTypeHeader.first.empty())
	{
		env.setVar("CONTENT_TYPE", contentTypeHeader.second);
	}
	else
	{
		env.setVar("CONTENT_TYPE", "");
	}
	std::pair<std::string, std::string> contentLengthHeader = request.getSingleHeader("content-length");
	if (!contentLengthHeader.first.empty())
	{
		env.setVar("CONTENT_LENGTH", contentLengthHeader.second);
		Debug::log("Content-Length header found.", Debug::NORMAL);
	}
	else
	{
		Debug::log("Content-Length header not found.", Debug::NORMAL);
	}
}

MetaVariables::~MetaVariables()
{
	// Since the function uses std::vector<std::string>, memory management is handled by the standard library.
	// The vectors are automatically destroyed once the function ends, and their contents are released.
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

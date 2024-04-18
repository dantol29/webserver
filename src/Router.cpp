#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::~Router()
{
}

HTTPResponse Router::routeRequest(const HTTPRequest &request)
{
	std::string _webRoot = "var/www"; // TODO: get this from the config file
	if (isCGI(request))
	{
		CGIHandler cgiHandler;
		return cgiHandler.handleRequest(request);
	}
	else if (isDynamicRequest(request))
	{
		std::cout << "\033[31mCGI is the only dynamic requests we handle at the moment\033[0m" << std::endl;
	}
	else // it is a static request
	{
		StaticContentHandler staticContentInstance;
		if (!pathIsValid(const_cast<HTTPRequest &>(request), _webRoot))
		{

			std::cout << "Path does not exist" << std::endl;
			return staticContentInstance.handleNotFound();
		}
		else
		{
			return staticContentInstance.handleRequest(request);
		}
	}
	return HTTPResponse();
}

bool Router::isDynamicRequest(const HTTPRequest &request)
{
	if (request.getMethod() == "POST" || request.getMethod() == "DELETE")
	{
		return true;
	}
	return false;
}

std::string Router::getFileExtension(const std::string &fileName)
{
	size_t dotIndex = fileName.find_last_of(".");
	if (dotIndex == std::string::npos)
	{
		return "";
	}

	size_t queryStart = fileName.find("?", dotIndex);
	if (queryStart == std::string::npos)
	{
		return fileName.substr(dotIndex + 1);
	}
	else
	{
		return fileName.substr(dotIndex + 1, queryStart - dotIndex - 1);
	}
}

bool Router::isCGI(const HTTPRequest &request)
{
	// TODO: check against config file, not this hardcoded version
	std::string fileExtension = getFileExtension(request.getRequestTarget());
	return (fileExtension == "cgi" || fileExtension == "pl" || fileExtension == "py" || fileExtension == "php");
}

void Router::splitTarget(const std::string &target)
{
	_path.directories.clear();
	_path.resource.clear();

	// TODO: Eventually remove if already done in the request
	std::string::size_type queryPos = target.find('?');
	std::string path = target.substr(0, queryPos);

	std::string::size_type start = 0, end = 0;

	while ((end = path.find('/', start)) != std::string::npos)
	{
		std::string token = path.substr(start, end - start);
		if (!token.empty())
		{
			_path.directories.push_back(token);
		}
		start = end + 1;
	}
	std::string lastToken = path.substr(start);
	if (!lastToken.empty())
	{
		_path.resource = lastToken;
	}
}

bool Router::pathIsValid(HTTPRequest &request, std::string webRoot)
{
	std::string host = request.getHost();
	std::cout << "Host: " << host << std::endl;
	size_t pos = host.find(":");
	if (pos != std::string::npos)
	{
		host = host.substr(0, pos);
	}
	std::cout << "Host (after : trailing) :" << host << std::endl;
	std::string path = request.getRequestTarget();
	path = webRoot + path;
	std::cout << "Path: " << path << std::endl;
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
	{
		return false;
	}
	if (S_ISDIR(buffer.st_mode))
	{
		std::cout << "Path is a directory" << std::endl;
		if (!path.empty() && path[path.length() - 1] != '/')
		{
			path += "/";
		}
		path += "index.html";
		std::cout << "Path: " << path << std::endl;
		if (stat(path.c_str(), &buffer) != 0)
		{
			// TODO: decide if we should return a custom error for a directory without an index.html
			return false;
		}
	}
	std::cout << "Path: " << path << " exists" << std::endl;

	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cout << "Failed to open the file at path: " << path << std::endl;
		return false;
	}
	file.close();

	std::cout << "Path is an accesible and readable file" << std::endl;
	return true;
}

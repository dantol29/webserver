#include "Router.hpp"
#include <string>

Router::Router() : _staticContentHandler(), _cgiHandler()
{
}

Router::~Router()
{
}

HTTPResponse Router::routeRequest(const HTTPRequest &request)
{
	HTTPResponse response;
	if (isDynamicRequest(request))
	{
		response = _cgiHandler.handleRequest(request);
	}
	else
	{
		response = _staticContentHandler.handleRequest(request);
	}
	return response;
}

bool Router::isDynamicRequest(const HTTPRequest &request)
{
	if (request.getMethod() == "POST" || request.getMethod() == "DELETE")
	{
		return true;
	}
	std::string fileExtension = getFileExtension(request.getRequestTarget());
	if (fileExtension == "cgi" || fileExtension == "php" || fileExtension == "py" || fileExtension == "pl")
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
		return ""; // No file extension
	}
	return fileName.substr(dotIndex + 1);
}

void Router::splitTarget(const std::string &target)
{
	_path.directories.clear();
	_path.resource.clear();

	// First, check for and remove any query parameters
	// TODO: Eventually remove if already done in the request
	std::string::size_type queryPos = target.find('?');
	std::string path = target.substr(0, queryPos); // If '?' is not found, substr returns the entire string

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

bool Router::pathExists(HTTPResponse &response, const std::string &path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
	{
		response.setStatusCode(404, "Not Found");
		response.setBody("Not Found");
		return false;
	}
	if (S_ISDIR(buffer.st_mode))
	{
		if (path.back() != '/')
		{
			path += '/';
		}
		path += "index.html";
		// Check if index.html exists
		if (stat(path.c_str(), &buffer) != 0)
		{
			response.setStatusCode(404, "Not Found");
			response.setBody("Not Found");
			return false;
		}
	}
	return true;
}
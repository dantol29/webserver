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
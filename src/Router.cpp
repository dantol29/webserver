#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::Router(Directives& directive) : _directive(directive), _FDsRef(NULL), _pollFd(NULL)
{
}

Router::Router(const Router &obj) : _directive(obj._directive), _path(obj._path), _FDsRef(NULL), _pollFd(NULL)
{
}

Router &Router::operator=(const Router &obj)
{
	if (this == &obj)
		return *this;
	_directive = obj._directive;
	_path = obj._path;
	_FDsRef = obj._FDsRef;
	_pollFd = obj._pollFd;
	return *this;
}

Router::~Router()
{
}

// Modifies the HTTP request path:  removes host header and "http://" prefix
// combines with root directory for standardized routing,
void Router::adaptPathForFirefox(HTTPRequest &request)
{
	std::string path = request.getRoot() + request.getSingleHeader("host").second;
	std::string requestTarget = request.getRequestTarget();
	size_t hostPos = requestTarget.find(request.getSingleHeader("host").second);
	if (hostPos != std::string::npos)
	{
		requestTarget.erase(hostPos, request.getSingleHeader("host").second.length());
	}

	size_t hostPos2 = requestTarget.find("http://");
	if (hostPos2 != std::string::npos)
	{
		std::string remove = "http://";
		requestTarget.erase(hostPos2, remove.length());
	}
	request.setRequestTarget(requestTarget);
	path += requestTarget;
	request.setPath(path);
}

void Router::routeRequest(HTTPRequest &request, HTTPResponse &response)
{
	Debug::log("Routing Request: host = " + request.getSingleHeader("host").second, Debug::NORMAL);
	
	if (!_directive._return.empty())
	{
		response.setStatusCode(301, "Redirection");
		response.setHeader("Location", _directive._return);
		return;
	}

	std::string root = _directive._root;
	if (root.empty())
		root = "var/";
	request.setRoot(root);
	std::string path = root + request.getSingleHeader("host").second;
	std::string requestTarget = request.getRequestTarget();
	std::cout << YELLOW << "requestTarget: " << requestTarget << RESET << std::endl;

	adaptPathForFirefox(request);

	std::cout << GREEN << "Routing request to path: " << request.getPath() << RESET << std::endl;

	// std::cout << request << std::endl;

	PathValidation pathResult = pathIsValid(response, request);
	std::cout << BLUE << "path: " << request.getPath() << RESET << std::endl;
	std::cout << BLUE << "PathValidation: " << pathResult << RESET << std::endl;
	switch (pathResult)
	{
	case PathValid:
		// check if method is allowed
		if (!_directive._allowedMethods.empty())
		{		
			for (size_t i = 0; i < _directive._allowedMethods.size(); i++)
			{
				if (_directive._allowedMethods[i] == request.getMethod())
				{
					break;
				}
				if (i == _directive._allowedMethods.size() - 1)
				{
					response.setStatusCode(405, "Method Not Allowed");
					handleServerBlockError(request, response, 405);
					return;
				}
			}
		}
		if (isCGI(request))
		{
			CGIHandler cgiHandler;
			cgiHandler.setFDsRef(_FDsRef);
			cgiHandler.setPollFd(_pollFd);
			cgiHandler.handleRequest(request, response);
		}
		else if (request.getMethod() == "POST" || request.getUploadBoundary() != "")
		{
			UploadHandler uploadHandler;
			uploadHandler.handleRequest(request, response);
		}
		else
		{
			StaticContentHandler staticContentHandler;
			staticContentHandler.handleRequest(request, response);
		}
		break;
	case IsDirectoryListing:
		std::cout << "Path is a directory listing, generating directory listing" << std::endl;
		generateDirectoryListing(response, request.getPath(), request.getRequestTarget());
		break;
	case PathInvalid:
		std::cout << "Path is not valid, handling as error" << std::endl;
		handleServerBlockError(request, response, 404);
		break;
	}

	if (request.getMethod() == "SALAD")
	{
		std::cout << "🥬 + 🍅 + 🐟 = 🥗" << std::endl;
	}
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
		return fileName.substr(dotIndex);
	}
	else
	{
		return fileName.substr(dotIndex, queryStart - dotIndex - 1);
	}
}

void Router::handleServerBlockError(HTTPRequest &request, HTTPResponse &response, int errorCode)
{
	Debug::log("handleServerBlockError: entering function", Debug::NORMAL);
	// clang-format off
	std::vector<std::pair<int, std::string> > errorPage = _directive._errorPage;
	// clang-format on
	size_t i = 0;
	for (; i < errorPage.size(); i++)
	{
		if (errorPage[i].first == errorCode)
		{
			std::cout << "handleServerBlockError: Error code: " << errorCode << std::endl;
			Debug::log("Path requested: " + request.getPath(), Debug::NORMAL);
			Debug::log("Path to error: " + errorPage[i].second, Debug::NORMAL);
			// setting the path to the custom error page
			request.setPath(_directive._root + request.getHost() + "/" + errorPage[i].second);
			// std::cout << RED << "         custom error page: " << request.getPath() << RESET << std::endl;
			// TODO: move here what is todo below
			StaticContentHandler staticContentInstance;
			staticContentInstance.handleRequest(request, response);
			response.setStatusCode(errorCode, errorPage[i].second);
			return;
		}
	}
	Debug::log("handleServerBlockError: No custom error page found", Debug::NORMAL);
	StaticContentHandler staticContentInstance;
	// if (errorCode == 404) // we have our own
	// {
	// 	staticContentInstance.handleNotFound(response);
	// }
	// else // we generate it
	// {
	response.setErrorResponse(errorCode);
	// }
	return;

	// TODO: below will not be executed, doublecheck and cleanup

	PathValidation pathResult = pathIsValid(response, request);
	switch (pathResult)
	{
	case PathValid:
		staticContentInstance.handleRequest(request, response);
		response.setStatusCode(errorCode, errorPage[i].second);
		break;
	case PathInvalid:
		Debug::log("handleServerBlockError: path to given error is not valid", Debug::NORMAL);
		response.setErrorResponse(errorCode);
		break;
	case IsDirectoryListing:
		Debug::log("handleServerBlockError: path to given error is a directory listing", Debug::NORMAL);
		break;
	}
}

bool Router::isCGI(const HTTPRequest &request)
{
	// TODO: check against config file, not this hardcoded version
	std::vector<std::string> cgiExtensions = _directive._cgiExt;
	std::cout << RED << "isCGI" << RESET << std::endl;
	std::cout << "cgiExtensions: " << cgiExtensions.size() << std::endl;
	std::cout << "request target: " << request.getRequestTarget() << std::endl;
	if (!cgiExtensions.empty())
	{
		std::string fileExtension = getFileExtension(request.getRequestTarget());
		std::cout << "fileExtension: " << fileExtension << std::endl;
		for (size_t i = 0; i < cgiExtensions.size(); i++)
		{
			std::cout << "cgiExtensions[" << i << "]: " << cgiExtensions[i] << std::endl;
			if (cgiExtensions[i] == fileExtension)
			{
				Debug::log("isCGI: CGI request detected", Debug::NORMAL);
				return true;
			}
		}
	}
	Debug::log("isCGI: Not a CGI request", Debug::NORMAL);
	return false;
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

void Router::generateDirectoryListing(HTTPResponse &Response,
									  const std::string &directoryPath,
									  const std::string &requestedPath)
{
	std::ostringstream html;
	html << "<html><head><title>Directory listing for " << requestedPath << "</title></head>"
		 << "<body><h1>Directory listing for " << requestedPath << "</h1><ul>";

	DIR *dir = opendir(directoryPath.c_str());
	if (dir)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name(entry->d_name);
			if (name == "." || name == "..")
				continue;

			std::string fullUrl = requestedPath;
			if (!requestedPath.empty() && requestedPath[requestedPath.length() - 1] != '/')
			{
				fullUrl += '/';
			}
			fullUrl += name;

			html << "<li><a href=\"" << fullUrl << "\">" << name << "</a></li>";
		}
		closedir(dir);
	}

	html << "</ul></body></html>";
	Response.setBody(html.str());
	Response.setStatusCode(200, "OK");
	Response.setHeader("Content-Type", "text/html");
}

bool isDirectory(std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    }
    return false; // Failed to get file information
}

enum PathValidation Router::pathIsValid(HTTPResponse &response, HTTPRequest &request)
{
	(void)response;
	struct stat buffer;
	std::string path = request.getPath();
	if (!isDirectory(path) && stat(path.c_str(), &buffer) == 0)
	{
		Debug::log("pathIsValid: stat success", Debug::NORMAL);
		std::cout << " path :" << path << std::endl;
		return PathValid;
	}
	else if (!isDirectory(path) && stat(path.c_str(), &buffer) != 0)
	{
		std::cout << "Failed to stat the file at path: " << path << std::endl;
		Debug::log("webRoot: " + path, Debug::NORMAL);
		Debug::log("pathIsValid: stat failed, path does not exist", Debug::NORMAL);
		return PathInvalid;
	}

	if (isDirectory(path))
	{
		Debug::log("pathIsValid: path is a directory", Debug::NORMAL);
		// if user provided index in config
		if (!path.empty() && !_directive._index.empty())
		{
			for (size_t i = 0; i < _directive._index.size(); i++)
			{
				std::string index = _directive._index[i];
				std::string tmpPath = request.getPath();
				tmpPath = tmpPath + "/" + index;
				std::cout << "tmpPath: " << tmpPath << std::endl;
				if (stat(tmpPath.c_str(), &buffer) == 0)
				{
					if (tmpPath.find("//") != std::string::npos)
						tmpPath.replace(tmpPath.find("//"), 2, "/");

					std::cout << "tmpPath: " << tmpPath << std::endl;
					Debug::log("pathIsValid: using index from user: " + index, Debug::NORMAL);
					request.setPath(tmpPath);
					return PathValid;
				}
			}
		}
		// if user did not provide any index in config or index is not valid
		if (!path.empty())
		{
			path += "/index.html"; // append /index.html
			std::cout << "path: " << path << std::endl;
			if (stat(path.c_str(), &buffer) == 0)
			{
				Debug::log("pathIsValid: using default index.html", Debug::NORMAL);
				request.setPath(path);
				return PathValid;
			}
		}
		// if autoindex is on
		if (_directive._autoindex)
		{
			Debug::log("pathIsValid: Autoindex is on", Debug::NORMAL);
			generateDirectoryListing(response, path, request.getRequestTarget());
			std::cout << "Directory listing generated for " << path << std::endl;
			return IsDirectoryListing;
		}
		Debug::log("pathIsValid: invalid path", Debug::NORMAL);
		return PathInvalid;
	}
	request.setPath(path);
	return PathValid;
}

void Router::setFDsRef(std::vector<struct pollfd> *FDsRef)
{
	_FDsRef = FDsRef;
}

void Router::setPollFd(struct pollfd *pollFd)
{
	_pollFd = pollFd;
}

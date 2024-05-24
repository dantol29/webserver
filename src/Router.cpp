#include "Router.hpp"
#include <string>

Router::Router(Directives &directive, EventManager &eventManager, Connection &connection)
	: _connection(connection)
	, _directive(directive)
	, _cgiHandler(eventManager, connection)
	, _eventManager(eventManager)
	, _FDsRef(NULL)
	, _pollFd(NULL)
{
	// Constructor body, if needed
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
	// remove trailing slash
	if (!requestTarget.empty() && requestTarget[requestTarget.length() - 1] == '/')
		requestTarget = requestTarget.substr(0, requestTarget.length() - 1);

	// remove port number
	if (requestTarget.find(':') != std::string::npos)
		requestTarget = requestTarget.substr(0, requestTarget.find(':'));

	request.setRequestTarget(requestTarget);
	path += requestTarget;
	request.setPath(path);
}

void Router::routeRequest(HTTPRequest &request, HTTPResponse &response)
{
	Debug::log("Routing Request: host = " + request.getSingleHeader("host").second, Debug::NORMAL);

	// in case of redirection
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

	adaptPathForFirefox(request);
	
	Debug::log("Routing Request: path = " + request.getPath(), Debug::NORMAL);

	PathValidation pathResult = pathIsValid(response, request);
	Debug::log("Routing Request: pathResult = " + toString(pathResult), Debug::NORMAL);
	Debug::log("Path requested: " + request.getPath(), Debug::NORMAL);
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

	switch (pathResult)
	{
	case PathValid:
		if (requestIsCGI(request) && !_connection.getHasCGI())
		{
			CGIHandler cgiHandler(_eventManager, _connection);
			cgiHandler.setFDsRef(_FDsRef);
			cgiHandler.setPollFd(_pollFd);
			cgiHandler.handleRequest(request, response);
			Debug::log("CGI request handled", Debug::CGI);
		}
		else if (request.getMethod() == "POST" || request.getUploadBoundary() != "")
		{
			UploadHandler uploadHandler;
			uploadHandler.setUploadDir(_directive._uploadPath);
			uploadHandler.handleRequest(request, response);
		}
		else
		{
			std::cout << "Path is a static content, handling as static content" << std::endl;
			StaticContentHandler staticContentHandler;
			staticContentHandler.handleRequest(request, response);
		}
		break;
	case IsDirectoryListing:
		generateDirectoryListing(response, request.getPath(), request.getRequestTarget());
		break;
	case PathInvalid:
		handleServerBlockError(request, response, 404);
		return;
	}

	if (request.getMethod() == "SALAD")
	{
		std::cout << "🥬 + 🍅 + 🐟 = 🥗" << std::endl;
	}

	if (response.getStatusCode() > 299)
		handleServerBlockError(request, response, response.getStatusCode());
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

bool Router::requestIsCGI(const HTTPRequest &request)
{
	// TODO: check against config file, not this hardcoded version
	std::vector<std::string> cgiExtensions = _directive._cgiExt;

	Debug::log("cgiExtensions: " + toString(cgiExtensions.size()), Debug::CGI);
	if (!cgiExtensions.empty())
	{
		std::string fileExtension = getFileExtension(request.getRequestTarget());
		Debug::log("fileExtension: " + fileExtension, Debug::CGI);
		for (size_t i = 0; i < cgiExtensions.size(); i++)
		{
			Debug::log("cgiExtensions[" + toString(i) + "]: " + cgiExtensions[i], Debug::CGI);
			if (cgiExtensions[i] == fileExtension)
			{
				Debug::log("requestIsCGI: CGI request detected", Debug::CGI);
				return true;
			}
		}
	}
	Debug::log("requestIsCGI: Not a CGI request", Debug::CGI);
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

bool isDirectory(std::string &path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		return S_ISDIR(buffer.st_mode);
	}
	return false; // Failed to get file information
}

enum PathValidation Router::pathIsValid(HTTPResponse &response, HTTPRequest &request)
{
	(void)response;
	struct stat buffer;
	std::string path = request.getPath();

	if (request.getUploadBoundary() != "")
		return PathValid;

	if (!isDirectory(path) && stat(path.c_str(), &buffer) == 0)
	{
		Debug::log("pathIsValid: stat success", Debug::NORMAL);
		Debug::log("pathIsValid: " + path, Debug::NORMAL);
		return PathValid;
	}
	else if (!isDirectory(path) && stat(path.c_str(), &buffer) != 0)
	{
		Debug::log("pathIsValid: stat failed: " + path, Debug::NORMAL);
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
				Debug::log("user error path: " + tmpPath, Debug::NORMAL);
				if (stat(tmpPath.c_str(), &buffer) == 0)
				{
					if (tmpPath.find("//") != std::string::npos)
						tmpPath.replace(tmpPath.find("//"), 2, "/");

					Debug::log("user error path: " + tmpPath, Debug::NORMAL);
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
			Debug::log("pathIsValid: " + path, Debug::NORMAL);
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
			Debug::log("pathIsValid: generated directory listing for " + path, Debug::NORMAL);
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

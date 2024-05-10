#include "Router.hpp"
#include <string>

Router::Router()
{
}

Router::Router(ServerBlock serverBlock) : _serverBlock(serverBlock), _FDsRef(NULL), _pollFd(NULL)
{
}

Router::Router(const Router &obj) : _serverBlock(obj._serverBlock), _path(obj._path), _FDsRef(NULL), _pollFd(NULL)
{
}

Router &Router::operator=(const Router &obj)
{
	if (this == &obj)
		return *this;
	_serverBlock = obj._serverBlock;
	_path = obj._path;
	_FDsRef = obj._FDsRef;
	_pollFd = obj._pollFd;
	return *this;
}

Router::~Router()
{
}

void Router::routeRequest(HTTPRequest &request, HTTPResponse &response)
{
	Debug::log("routeRequest Request host: " + request.getSingleHeader("host").second, Debug::NORMAL);
	Debug::log("routeRequest _webRoot: " + _serverBlock.getRoot(), Debug::NORMAL);
	std::string _webRoot = _serverBlock.getRoot();
	Debug::log("routeRequest _webRoot: " + _webRoot, Debug::NORMAL);
	request.setRoot(_webRoot);
	_webRoot += request.getSingleHeader("host").second;
	Debug::log("_webRoot += request.getSingleHeader(host).second: " + _webRoot, Debug::NORMAL);

	request.setPath(_webRoot);

	std::string requestTarget = request.getRequestTarget();
	std::cout << GREEN << "routeRequest: requestTarget " << requestTarget << RESET << std::endl;

	_webRoot += requestTarget;
	std::cout << GREEN << "routeRequest: _webRoot " << _webRoot << RESET << std::endl;

	request.setPath(_webRoot);
	std::cout << GREEN << "routeRequest: request.getPath() " << request.getPath() << RESET << std::endl;

	if (isCGI(request) && pathIsValid(const_cast<HTTPRequest &>(request)))
	{
		CGIHandler cgiHandler;
		cgiHandler.setFDsRef(_FDsRef);
		cgiHandler.setPollFd(_pollFd);
		cgiHandler.handleRequest(request, response);
		return;
	}
	else if (request.getMethod() == "POST") // && !request.getUploadBoundary().empty())
	{
		std::cout << "Router: POST request" << std::endl;

		UploadHandler uploadHandler;
		uploadHandler.handleRequest(request, response);
	}
	else if (request.getMethod() == "SALAD")
	{
		std::cout << "🥬 + 🍅 + 🐟 = 🥗" << std::endl;
	}
	else
	{
		StaticContentHandler staticContentInstance;
		if (!pathIsValid(const_cast<HTTPRequest &>(request)))
		{
			std::cout << "Path is not valid, calling handleNotFound" << std::endl;
			handleServerBlockError(request, response, 400);
			// staticContentInstance.handleNotFound(response);
		}
		else
		{
			staticContentInstance.handleRequest(request, response);
		}
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
		return fileName.substr(dotIndex + 1);
	}
	else
	{
		return fileName.substr(dotIndex + 1, queryStart - dotIndex - 1);
	}
}

void Router::handleServerBlockError(HTTPRequest &request, HTTPResponse &response, int errorCode)
{
	// clang-format off
	std::vector<std::pair<int, std::string> > errorPage = _serverBlock.getErrorPage();
	// clang-format on
	std::string errorPath;
	size_t i = 0;
	for (; i < errorPage.size(); i++)
	{
		if (errorPage[i].first == errorCode)
		{
			std::cout << "handleServerBlockError: Error code: " << errorCode << std::endl;
			Debug::log("Path requested: " + request.getPath(), Debug::NORMAL);
			Debug::log("Path to error: " + errorPage[i].second, Debug::NORMAL);
			request.setPath(_serverBlock.getRoot() + request.getHost() + errorPage[i].second);
			break;
		}
	}
	errorPath = request.getPath();
	// if (errorPath.empty())
	// {
	// 	std::cout << "handleServerBlockError: Error path is empty" << std::endl;
	// 	return;
	// }
	StaticContentHandler staticContentInstance;
	if (!pathIsValid(const_cast<HTTPRequest &>(request)))
	{
		Debug::log("handleServerBlockError: path to given error is not valid", Debug::NORMAL);
		staticContentInstance.handleNotFound(response);
	}
	else
	{
		Debug::log("handleServerBlockError: Response set to custom error file", Debug::NORMAL);
		staticContentInstance.handleRequest(request, response);
		response.setStatusCode(errorCode, errorPage[i].second);
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

std::string Router::generateDirectoryListing(const std::string &directoryPath, const std::string &requestedPath)
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
	return html.str();
}

bool Router::pathIsValid(HTTPRequest &request)
{
	struct stat buffer;
	std::string path = request.getPath();
	if (stat(path.c_str(), &buffer) != 0)
	{
		Debug::log("webRoot: " + path, Debug::NORMAL);
		Debug::log("pathIsValid: stat failed, path does not exist", Debug::NORMAL);
		return false;
	}
	if (S_ISDIR(buffer.st_mode))
	{
		// TODO: finish implementing this temporary directory listing output
		std::string directoryListing = generateDirectoryListing(path, request.getRequestTarget());
		std::cout << "Directory listing: " << directoryListing << std::endl;

		if (!path.empty() && path[path.length() - 1] != '/')
		{
			path += "/";
		}
		if (_serverBlock.getIndex().empty())
		{
			Debug::log("User did not provided any index", Debug::NORMAL);
			// if directory listing
			// if (_serverBlock.getAutoindex())
			// {
			// 	Debug::log("pathIsValid: Autoindex is on", Debug::NORMAL);
			// 	// path += "index.html";
			// 	// request.setPath(path);
			// 	// std::cout << std::endl;
			// }
			// else
			// {
			// 	Debug::log("pathIsValid: Autoindex is off", Debug::NORMAL);
			// 	return false;
			// }
		}
		else // user provided an index
		{
			Debug::log("pathIsValid: Index: " + _serverBlock.getIndex()[0], Debug::NORMAL);
			// TODO: implement several indexes
			std::string index = _serverBlock.getIndex()[0];
			Debug::log("pathIsValid: Index: " + index, Debug::NORMAL);
			path += index;
			Debug::log("pathIsValid: path: " + path, Debug::NORMAL);
		}
		request.setPath(path);
		std::cout << std::endl;
	}

	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		std::cout << "Failed to open the file at path: " << path << std::endl;

		// if (path[path.length() - 1] != '/')
		// {
		// 	path += "/";
		// }
		// path += _serverBlock.getErrorPage()[0].second;
		// std::cout << GREEN << "pathIsValid: path: " << path << RESET << std::endl;
		return false;
	}
	file.close();

	return true;
}

void Router::setFDsRef(std::vector<struct pollfd> *FDsRef)
{
	_FDsRef = FDsRef;
}

void Router::setPollFd(struct pollfd *pollFd)
{
	_pollFd = pollFd;
}

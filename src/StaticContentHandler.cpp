#include "StaticContentHandler.hpp"
#include "Server.hpp"
StaticContentHandler::StaticContentHandler()
{
}

StaticContentHandler::~StaticContentHandler()
{
}

bool endsWith(const std::string &str, const std::string &suffix)
{
	if (str.length() >= suffix.length())
	{
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
	}
	else
	{
		return false;
	}
}
std::string getMimeType(const std::string &filePath)
{
	if (endsWith(filePath, ".html") || endsWith(filePath, ".htm"))
		return "text/html";
	else if (endsWith(filePath, ".css"))
		return "text/css";
	else if (endsWith(filePath, ".js"))
		return "application/javascript";
	else if (endsWith(filePath, ".json"))
		return "application/json";
	else if (endsWith(filePath, ".jpg") || endsWith(filePath, ".jpeg"))
		return "image/jpeg";
	else if (endsWith(filePath, ".png"))
		return "image/png";
	// TODO: checks for other file types and dynamic content scripts
	else
		return "application/octet-stream"; // Default binary type
}

// static bool isDirectory(const std::string &path)
// {
// 	struct stat statbuf;
// 	if (stat(path.c_str(), &statbuf) != 0)
// 		return false;
// 	return S_ISDIR(statbuf.st_mode);
// }

void StaticContentHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	std::string webRoot = "var/www";
	std::string host = request.getHost();

	std::string path = request.getPath();
	std::cout << "path: " << path << std::endl;
	
	// this is totally wrong
	// if (requestTarget == "/" || requestTarget == "")
	// 	requestTarget = "/index.html";
	
	// is this necessary?
	// if the last character of the path is a / and the first character of the request target is a /, we remove the
	// first character of the request target
	// std::cout << "requestTarget: " << request.getRequestTarget() << std::endl;
	// if (path[path.length() - 1] == '/' && request.getRequestTarget()[0] == '/')
	// 	request.setRequestTarget(request.getRequestTarget().substr(1));
	
	// TODO: consider streaming the file instead of loading it all in memory for large files
	
	// this is totally wrong
	// if (isDirectory(path))
	// {
	// 	path += "index.html";
	// }

	std::ifstream file(path.c_str());
	if (!file) // TODO: this is wrong, it should return a false bool
	{
		Debug::log(" StaticContentHandler Error opening file: " + path, Debug::NORMAL);
		response.setStatusCode(404, "Not Found");
		response.setBody("404 Not Found");
		return;
	}
	std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	// TODO : take code arg if we serve an error page
	response.setStatusCode(200, "OK");
	response.setBody(body);
	response.setHeader("Content-Type", getMimeType(path));
	response.setHeader("Content-Length", toString(body.length()));
	response.setHeader("Set-Cookie", "meal=salad; Expires=Wed, 09 Jun 2024 10:18:14 GMT; Path=/;");
	response.setHeader("Set-Cookie", "user=dtolmaco; Expires=Wed, 09 Jun 2024 10:18:14 GMT; Path=/;");
	response.setStatusCode(200, "");
	// TODO ADD MORE HEADER LINE
	//  response.setHeader("Content-Length: ", std::to_string(body.length()));
	//  response.setHeader("Connection: ", "close");
	//  response.setHeader("Server: ", "webserv");

	// std::cout << std::endl;
	// std::cout << "_body : " << response.getBody() << std::endl;
	file.close();
	return;
}

void StaticContentHandler::handleNotFound(HTTPResponse &response)
{
	Debug::log("StaticContentHandler: sending default 404 Not Found", Debug::NORMAL);
	std::ifstream file("html/errors/404.html");
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fileContents = buffer.str();

	response.setBody(fileContents);
	response.setHeader("Content-Type", "text/html");
	response.setHeader("Content-Length", toString(fileContents.length()));
	response.setStatusCode(400, "");
	return;
}

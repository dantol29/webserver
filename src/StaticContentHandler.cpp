#include "StaticContentHandler.hpp"
#include "Server.hpp"
// Default constructor
StaticContentHandler::StaticContentHandler()
{
}
// Constructor
StaticContentHandler::StaticContentHandler(const std::string &webRoot) : _webRoot(webRoot) {};

// Destructor
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
	// Add checks for other file types and dynamic content scripts
	else
		return "application/octet-stream"; // Default binary type
}

// Handle request
HTTPResponse StaticContentHandler::handleRequest(const HTTPRequest &request)
{
	HTTPResponse response;
	std::string requestTarget = request.getRequestTarget();
	std::string path = _webRoot + requestTarget;
	// Eventually sanitze path, remove ".." and other dangerous characters
	std::ifstream file(path);
	if (file.is_open())
	{
		// TODO: consider streaming the file instead of loading it all in memory for large files
		std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		response.setStatusCode(200);
		response.setBody(body);
		response.setHeader("Content-Type", getMimeType(path));
	}
	else
	{
		// TODO: consider serving a 404.html page
		response.setStatusCode(404);
		response.setBody("Not Found");
		response.setHeader("Content-Type", "text/plain");
	}
	return response;
}

// This should return a HTTPResponse object
// std::string handleHomePage()
// {
// 	std::string htmlContent = readHtml("./html/home.html");
// 	std::stringstream ss;
// 	ss << htmlContent.length();
// 	std::string htmlLength = ss.str();
// 	std::string httpResponse = "HTTP/1.1 200 OK\nContent-Type: text/html\n" + std::string("Content-Length: ") +
// 							   htmlLength + "\n\n" + htmlContent;
// 	std::cout << "------------------Home page returned from handleHomePage()-------------------" << std::endl;
// 	return httpResponse;
// }
HTTPResponse StaticContentHandler::handleHomePage()
{
	std::string htmlContent = readHtml("./html/home.html");
	HTTPResponse response;
	response.setStatusCode(200);
	response.setBody(htmlContent);
	response.setHeader("Content-Type", "text/html");
	std::cout << "------------------Home page returned from handleHomePage()-------------------" << std::endl;
	return response;
}

// This should return a HTTPResponse object
// std::string handleNotFound(void)
// {
// 	std::string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
// 	// write(socket, response.c_str(), response.size());
// 	std::cout << "------------------404 Not Found sent-------------------" << std::endl;

// 	return response;
// }

HTTPResponse StaticContentHandler::handleNotFound(void)
{
	HTTPResponse response;
	response.setStatusCode(404);
	response.setBody("Not Found");
	response.setHeader("Content-Type", "text/plain");
	std::cout << "------------------404 Not Found sent-------------------" << std::endl;
	return response;
}
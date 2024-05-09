#include "StaticContentHandler.hpp"
#include "Server.hpp"
StaticContentHandler::StaticContentHandler()
{
}

StaticContentHandler::StaticContentHandler(const std::string &webRoot) : _webRoot(webRoot) {};

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

bool isDirectory(const std::string &path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

void StaticContentHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	std::string requestTarget = request.getRequestTarget();
	std::string webRoot = "var/www";
	// std::cout << "path in handleRequest: " << webRoot << std::endl;
	std::string host = request.getHost();
	// std::cout << "host in handleRequest: " << host << std::endl;

	std::string path;

	// for ease of use during deployment
	// this if/else allows to reach target with tester or browser
	if (host == "localhost:8080")
		path = webRoot + requestTarget;
	else
		path = webRoot + "/" + host + requestTarget;

	// std::string path = webRoot + "/" + host + requestTarget;

	std::cout << std::endl << "path : " << path << std::endl << std::endl;
	if (requestTarget == "/" || requestTarget == "")
		requestTarget = "/index.html";
	// TODO: consider streaming the file instead of loading it all in memory for large files
	if (isDirectory(path))
	{
		path += "/index.html";
	}
	std::cout << "path : " << path << std::endl;
	std::ifstream file(path.c_str());
	if (!file)
	{
		std::cerr << "Error opening file: " << path << std::endl;
		response.setStatusCode(404, "Not Found");
		response.setBody("404 Not Found");
		return;
	}

	std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	// std::cout << "body : " << body << std::endl;

	response.setStatusCode(200, "OK");
	response.setBody(body);
	response.setHeader("Content-Type", getMimeType(path));
	response.setHeader("Content-Length", toString(body.length()));
	response.setStatusCode(200, "");
	// TODO ADD MORE HEADER LINE
	//  response.setHeader("Content-Length: ", std::to_string(body.length()));
	//  response.setHeader("Connection: ", "close");
	//  response.setHeader("Server: ", "webserv");

	std::cout << std::endl;
	// std::cout << "_body : " << response.getBody() << std::endl;
	file.close();
	return;
}

void StaticContentHandler::handleNotFound(HTTPResponse &response)
{
	std::ifstream file("errors/404.html");
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fileContents = buffer.str();

	response.setBody(fileContents);
	response.setHeader("Content-Type", "text/html");
	response.setHeader("Content-Length", toString(fileContents.length()));
	response.setStatusCode(404, "");
	return;
}

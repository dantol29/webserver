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
	// Add checks for other file types and dynamic content scripts
	else
		return "application/octet-stream"; // Default binary type
}

HTTPResponse StaticContentHandler::handleRequest(const HTTPRequest &request)
{
	HTTPResponse response;
	std::string requestTarget = request.getRequestTarget();
	std::string webRoot = "var/www/";
	std::cout << "path : " << webRoot << std::endl;
	std::string path = webRoot + requestTarget;
	// shoul nt be hardcoded: (added underscore) std::string path = _webRoot + requestTarget;
	std::ifstream file(path.c_str());
	// check this: we might be at the root of a website among others
	if (request.getMethod() == "GET" && (request.getRequestTarget() == "/" || request.getRequestTarget() == "/home"))
	{
		response = handleHomePage();
	}
	else
	{
		// TODO: consider streaming the file instead of loading it all in memory for large files
		std::cout << "path : " << path << std::endl;
		std::ifstream file(path.c_str());
		file.is_open();

		std::string body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		std::cout << "body : " << body << std::endl;

		// response.setStatusCode(200);
		response.setBody(body);
		response.setHeader("Content-Type: ", getMimeType(path));
		// ADD MORE HEADER LINE
		//  response.setHeader("Content-Length: ", std::to_string(body.length()));
		//  response.setHeader("Connection: ", "close");
		//  response.setHeader("Server: ", "webserv");

		std::cout << "_body : " << response.getBody() << std::endl;
		file.close();
	}
	return response;
}

HTTPResponse StaticContentHandler::handleHomePage()
{
	std::string htmlContent = readHtml("./html/index.html");
	HTTPResponse response;
	response.setStatusCode(200);
	response.setBody(htmlContent);
	response.setHeader("Content-Type", "text/html");
	std::cout << "------------------Home page returned from handleHomePage()-------------------" << std::endl;
	return response;
}

HTTPResponse StaticContentHandler::handleNotFound(void)
{
	HTTPResponse response;
	response.setStatusCode(404);

	// Create a simple HTML body for the 404 page
	std::string htmlBody = "<!DOCTYPE html>\n"
						   "<html lang=\"en\">\n"
						   "<head>\n"
						   "    <meta charset=\"UTF-8\">\n"
						   "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
						   "    <title>404 Not Found</title>\n"
						   "    <style>\n"
						   "        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
						   "        h1 { color: #ff6347; }\n"
						   "        p { color: #757575; }\n"
						   "    </style>\n"
						   "</head>\n"
						   "<body>\n"
						   "    <h1>404 Not Found</h1>\n"
						   "    <p>Sorry, the page you're looking for cannot be found.</p>\n"
						   "</body>\n"
						   "</html>";

	response.setBody(htmlBody);
	response.setHeader("Content-Type", "text/html"); // Set the Content-Type to text/html
	std::cout << "------------------404 Not Found sent-------------------" << std::endl;
	return response;
}

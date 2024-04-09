#include "Server.hpp"
#include "webserver.hpp"
#include "server_utils.cpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "webserv.hpp"
#include "HTTPRequest.hpp"

const int BUFFER_SIZE = 1024;

void printVariablesHeadersBody(const HTTPRequest &obj)
{
	std::multimap<std::string, std::string> a = obj.getHeaders();
	std::multimap<std::string, std::string> b = obj.getQueryString();
	std::vector<std::string> c = obj.getBody();

	std::multimap<std::string, std::string>::iterator it;
	std::cout << "Variables: =>" << std::endl;
	for (it = b.begin(); it != b.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	std::cout << "Headers: =>" << std::endl;
	for (it = a.begin(); it != a.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	std::cout << "Body: =>" << std::endl;
	for (size_t i = 0; i < c.size(); ++i)
		std::cout << c[i] << std::endl;
}

// Determine the type of request and call the appropriate handler
void handleConnection(int socket)
{
	char buffer[BUFFER_SIZE] = {0};
	long valRead = read(socket, buffer, BUFFER_SIZE);
	HTTPRequest obj(buffer);
	if (obj.getIsChunked() == true)
		obj.parseChunkedBody(buffer);
	std::cout << obj.getStatusCode() << std::endl;
	printVariablesHeadersBody(obj);
	if (valRead < 0)
	{
		perror("In read");
		exit(EXIT_FAILURE);
	}
	std::cout << "Received HTTP request: " << std::endl << buffer << std::endl;

	// test to execute the python script (see: https://www.tutorialspoint.com/python/python_cgi_programming.htm)
	const char *argv[] = {"./cgi-bin/hello_py.cgi", NULL};
	// const char* argv[] = { "./cgi-bin/thirty_py.cgi", NULL };
	// const char* argv[] = { "./cgi-bin/hello.cgi", NULL };

	// std::string response;
	HTTPResponse response;
	Router router;
	if (!router.pathExists(response, obj.getRequestTarget()))
	{
		StaticContentHandler staticContentHandler;
		// This shoud be a method of the requestHandler obect
		// response = router.handleNotFound();
		response = staticContentHandler.handleNotFound();
	}
	else if (router.isDynamicRequest(obj))
	{
		if (obj.getMethod() == "GET" && obj.getRequestTarget() == "/hello")
		{
			// env has to be created before CGI, because it is passed to the CGI
			CGIHandler cgiHandler;
			Environment env;
			env.setVar("QUERY_STRING", "Hello from C++ CGI!");
			response = cgiHandler.handleCGIRequest(argv, env);
		}
		else
		{
			CGIHandler cgiHandler;
			Environment env;
			env.setVar(obj.getQueryString(), obj.getBody());
			response = cgiHandler.handleCGIRequest(argv, obj);
		}
	}
	else
	{
		StatcContentHandler staticContentHandler;
		// This if condition only for legacy reasons! TODO: remove
		if (obj.getMethod() == "GET" && (obj.getRequestTarget() == "/" || obj.getRequestTarget() == "/home")
		{
			response = staticContentHandler.handleHomePage();

		}
		else
		{
			response = staticContentHandler.handleRequest(obj);
		}
	}
	std::string responseString = response.toString();

	write(socket, responseString.c_str(), responseString.size());
	close(socket);
}

Server Server::getServer()
{
	return *this;
}

void Server::loadDefaultConfig()
{
	_webRoot = "var/www";
}

std::string Server::getWebRoot() const
{
	return _webRoot;
}

void Server::setWebRoot(const std::string &webRoot)
{
	_webRoot = webRoot;
}

std::string Server::getConfigFilePath() const
{
	return _configFilePath;
}

void Server::loadConfig()
{
	// Add logic to load config from file
}

void Server::start()
{
	// Add logic to start the server
}

void Server::stop()
{
	// Add logic to stop the server
}
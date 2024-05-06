#include "UploadHandler.hpp"
#include "Server.hpp"

UploadHandler::UploadHandler()
{
}

UploadHandler::UploadHandler(const std::string &webRoot) : _webRoot(webRoot) {};

UploadHandler::~UploadHandler()
{
}

void createFile(HTTPRequest &request)
{
	std::vector<File> files = request.getFiles();
	std::vector<File>::iterator it;

	for (it = files.begin(); it != files.end(); ++it)
	{
		if (it->headers.find("filename") == it->headers.end())
		{
			std::cout << "422 Unprocessable Entity (Error: file does not have a name)" << std::endl;
			return;
		}
	}

	for (it = files.begin(); it != files.end(); ++it)
	{
		std::ofstream outfile((it->headers.find("filename"))->second.c_str());
		if (outfile.is_open())
		{
			outfile << it->fileContent;
			outfile.close();
			std::cout << "File created successfully" << std::endl;
		}
		else
			std::cout << "422 Unprocessable Entity (Error creating a file)" << std::endl;
	}
}

void UploadHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	if (!request.getUploadBoundary().empty())
	{
		createFile(const_cast<HTTPRequest &>(request));
		handleResponse(response, "success");
	}
	else
	{
		std::cout << "415 Unsupported Media Type" << std::endl;
		handleResponse(response, "bad_request");
	}
	return;
}

std::string readFileContents(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "Could not open file: " << filePath << std::endl;
		return "<html><body><h1>Internal Server Error</h1></body></html>";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

// Possible codes:
// 413 Payload Too Large, 415 Unsupported Media, 401 Unauthorized,
// 403 Forbidden, 500 Internal Server Error
void UploadHandler::handleResponse(HTTPResponse &response, const std::string &code)
{
	std::string fileContents;
	int statusCode;
	std::string statusDescription;

	// ADD A MESSAGE WITH THE NAME OF THE FILE CREATED

	if (code == "success")
	{
		fileContents = readFileContents("errors/200_upload_success.html");
		statusCode = 200;
		statusDescription = "OK";
	}
	else if (code == "bad_request")
	{
		fileContents = readFileContents("errors/400.html");
		statusCode = 400;
		statusDescription = "Bad Request";
	}
	else
	{
		fileContents = "<html><body><h1>Unknown Error</h1></body></html>";
		statusCode = 500;
		statusDescription = "Internal Server Error";
	}

	response.setHeader("Content-Type", "text/html");
	response.setHeader("Content-Length", std::to_string(fileContents.length()));
	response.setBody(fileContents);
	response.setStatusCode(statusCode, statusDescription);
}
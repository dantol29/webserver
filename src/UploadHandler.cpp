#include "UploadHandler.hpp"
#include "Server.hpp"

UploadHandler::UploadHandler()
{
}

UploadHandler::UploadHandler(const std::string &webRoot) : _webRoot(webRoot) {};

UploadHandler::~UploadHandler()
{
}

bool UploadHandler::isHarmfulExtension(const std::string &extension)
{
	std::vector<std::string> executableFiles;
	executableFiles.push_back("exe");
	executableFiles.push_back("com");
	executableFiles.push_back("bat");
	executableFiles.push_back("cmd");
	executableFiles.push_back("msi");
	executableFiles.push_back("scr");
	executableFiles.push_back("pif");
	executableFiles.push_back("ps1");
	std::vector<std::string> dynamicLibraries;
	dynamicLibraries.push_back("dll");
	dynamicLibraries.push_back("so");
	std::vector<std::string> scripts;
	scripts.push_back("js");
	scripts.push_back("vbs");
	scripts.push_back("py");
	scripts.push_back("php");
	scripts.push_back("pl");
	scripts.push_back("sh");
	std::vector<std::string> configurationFiles;
	configurationFiles.push_back("ini");
	configurationFiles.push_back("inf");
	configurationFiles.push_back("reg");

	std::vector<std::string> harmfulExtensions;
	harmfulExtensions.insert(harmfulExtensions.end(), executableFiles.begin(), executableFiles.end());
	harmfulExtensions.insert(harmfulExtensions.end(), dynamicLibraries.begin(), dynamicLibraries.end());
	harmfulExtensions.insert(harmfulExtensions.end(), scripts.begin(), scripts.end());
	harmfulExtensions.insert(harmfulExtensions.end(), configurationFiles.begin(), configurationFiles.end());

	if (std::find(harmfulExtensions.begin(), harmfulExtensions.end(), extension) != harmfulExtensions.end())
	{
		return true;
	}
	return false;
}

// Other possible codes:
// 413 Payload Too Large, 401 Unauthorized, 403 Forbidden, 500 Internal Server Error
void UploadHandler::checkFiles(const HTTPRequest &request)
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
		if (it->fileContent.empty())
		{
			std::cout << "422 Unprocessable Entity (Error: file is empty)" << std::endl;
			return;
		}
	}
	for (it = files.begin(); it != files.end(); ++it)
	{

		std::string filename = it->headers.find("filename")->second;
		std::string extension = filename.substr(filename.find_last_of(".") + 1);
		if (extension.empty())
		{
			// TODO: have an html ?
			std::cout << "415 Unsupported Media Type (Error: file has no extension)" << std::endl;
			return;
		}
		if (isHarmfulExtension(extension))
		{
			std::cout << "415 Unsupported Media Type (Error: file has a harmful extension)" << std::endl;
			return;
		}
	}
}

void createFile(HTTPRequest &request)
{
	const std::string uploadDir = "upload/";

	std::vector<File> files = request.getFiles();
	std::vector<File>::iterator it;

	for (it = files.begin(); it != files.end(); ++it)
	{
		std::string filePath = uploadDir + (it->headers.find("filename"))->second;

		std::ofstream outfile(filePath.c_str());
		if (outfile.is_open())
		{
			outfile << it->fileContent;
			outfile.close();
			std::cout << "File created successfully at " << filePath << std::endl;
		}
		else
		{
			std::cout << "422 Unprocessable Entity (Error creating a file at " << filePath << ")" << std::endl;
		}
	}
}

void UploadHandler::handleRequest(const HTTPRequest &request, HTTPResponse &response)
{
	checkFiles(request);
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
}

std::string readFileContents(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file)
	{
		std::cerr << "Could not open file: " << filePath << std::endl;
		return "<html><body><h1>Internal Server Error</h1></body></html>";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void UploadHandler::handleResponse(HTTPResponse &response, const std::string &code)
{
	std::string fileContents;
	int statusCode;
	std::string statusDescription;

	if (code == "success")
	{
		std::cout << "Upload: File created successfully" << std::endl;
		fileContents = readFileContents("html/success/200_upload_success.html");
		statusCode = 200;
		statusDescription = "OK";
	}
	else if (code == "bad_request")
	{
		std::cout << "Upload: Bad request" << std::endl;
		fileContents = readFileContents("html/errors/400.html");
		statusCode = 400;
		statusDescription = "Bad Request";
	}
	else
	{
		std::cout << "Upload: Internal server error" << std::endl;
		fileContents = "<html><body><h1>Unknown Error</h1></body></html>";
		statusCode = 500;
		statusDescription = "Internal Server Error";
	}

	response.setHeader("Content-Type", "text/html");
	response.setHeader("Content-Length", toString(fileContents.length()));
	response.setBody(fileContents);
	response.setStatusCode(statusCode, statusDescription);
}
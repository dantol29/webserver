#include "UploadHandler.hpp"
#include "Server.hpp"

UploadHandler::UploadHandler()
{
}

UploadHandler::~UploadHandler()
{
}

void UploadHandler::setUploadDir(const std::string &uploadDir)
{
	_uploadDir = uploadDir;
}

bool UploadHandler::isHarmfulExtension(const std::string &extension)
{
	const char *executableFilesArr[] = {"exe", "com", "bat", "cmd", "msi", "scr", "pif", "ps1"};
	const char *dynamicLibrariesArr[] = {"dll", "so"};
	const char *scriptsArr[] = {"js", "vbs", "py", "php", "pl", "sh"};
	const char *configurationFilesArr[] = {"ini", "inf", "reg"};

	// Initialize vectors directly from arrays
	std::vector<std::string> executableFiles(
		executableFilesArr, executableFilesArr + sizeof(executableFilesArr) / sizeof(executableFilesArr[0]));
	std::vector<std::string> dynamicLibraries(
		dynamicLibrariesArr, dynamicLibrariesArr + sizeof(dynamicLibrariesArr) / sizeof(dynamicLibrariesArr[0]));
	std::vector<std::string> scripts(scriptsArr, scriptsArr + sizeof(scriptsArr) / sizeof(scriptsArr[0]));
	std::vector<std::string> configurationFiles(
		configurationFilesArr,
		configurationFilesArr + sizeof(configurationFilesArr) / sizeof(configurationFilesArr[0]));

	// Combine all vectors into a single vector
	std::vector<std::string> harmfulExtensions;
	harmfulExtensions.reserve(executableFiles.size() + dynamicLibraries.size() + scripts.size() +
							  configurationFiles.size());

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
bool UploadHandler::checkFiles(const HTTPRequest &request)
{
	std::vector<File> files = request.getFiles();
	std::vector<File>::iterator it;

	for (it = files.begin(); it != files.end(); ++it)
	{
		if (it->headers.find("filename") == it->headers.end())
		{
			std::cout << "422 Unprocessable Entity (Error: file does not have a name)" << std::endl;
			return false;
		}
	}
	for (it = files.begin(); it != files.end(); ++it)
	{
		if (it->fileContent.empty())
		{
			std::cout << "422 Unprocessable Entity (Error: file is empty)" << std::endl;
			return false;
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
			return false;
		}
		if (isHarmfulExtension(extension))
		{
			std::cout << "415 Unsupported Media Type (Error: file has a harmful extension)" << std::endl;
			return false;
		}
	}
	return true;
}

bool UploadHandler::createFile(HTTPRequest &request)
{
	if (_uploadDir.empty())
		_uploadDir = request.getRoot() + request.getHost();
	else
		_uploadDir = request.getRoot() + request.getHost() + "/" + _uploadDir;

	
	std::cout << "Creating file at " << _uploadDir << std::endl;
	std::vector<File> files = request.getFiles();
	std::vector<File>::iterator it;

	for (it = files.begin(); it != files.end(); ++it)
	{
		std::string filePath = _uploadDir + (it->headers.find("filename"))->second;
		std::cout << "Creating file at " << filePath << std::endl;
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
			return (false);
		}
	}
	return (true);
}

bool UploadHandler::createFileChunked(HTTPRequest &request)
{
	if (_uploadDir.empty())
		_uploadDir = request.getRoot() + request.getHost();
	else
		_uploadDir = request.getRoot() + request.getHost() + "/" + _uploadDir;
	std::string filepath = "chunked_upload.jpg";
	_uploadDir += filepath;

	std::ofstream outfile(_uploadDir.c_str());
	if (outfile.is_open())
	{
		outfile << request.getBody();
		outfile.close();
		std::cout << "File created successfully at " << _uploadDir << std::endl;
	}
	else
	{

		std::cout << "422 Unprocessable Entity (Error creating a file at " << _uploadDir << ")" << std::endl;
		return (false);
	}
	return (true);
}

void UploadHandler::handleRequest(HTTPRequest &request, HTTPResponse &response)
{
	if (!checkFiles(request))
	{
		std::cout << PURPLE << "calling handle response" << RESET << std::endl;
		handleResponse(response, BAD_REQUEST);
		return;
	}
	if (!request.getUploadBoundary().empty())
	{
		std::cout << PURPLE << "calling upload boundary" << RESET << std::endl;
		if (!createFile(const_cast<HTTPRequest &>(request)))
			response.setStatusCode(422, "Unprocessable Entity");
		handleResponse(response, SUCCESS);
	}
	else
	{
		// logic is incorrect here in case of chunked request
		// temporary solution
		std::cout << PURPLE << "calling create file chunked" << RESET << std::endl;
		if (!createFileChunked(const_cast<HTTPRequest &>(request)))
			response.setStatusCode(422, "Unprocessable Entity");
		handleResponse(response, SUCCESS);
		// std::cout << "415 Unsupported Media Type" << std::endl;
		// handleResponse(response, BAD_REQUEST);
	}
}

std::string UploadHandler::readFileContents(const std::string &filePath)
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

void UploadHandler::handleResponse(HTTPResponse &response, enum UploadStatus status)
{
	std::string fileContents;
	int statusCode;

	if (status == SUCCESS)
	{
		std::cout << "Upload: File created successfully" << std::endl;
		fileContents = readFileContents("html/success/200_upload_success.html");
		statusCode = 200;
		response.setStatusCode(statusCode, "OK");
	}
	else if (status == BAD_REQUEST)
	{
		std::cout << "Upload: Bad request" << std::endl;
		fileContents = readFileContents("html/errors/400.html");
		statusCode = 400;
		response.setStatusCode(statusCode, "Bad Request");
	}
	else
	{
		std::cout << "Upload: Internal server error" << std::endl;
		fileContents = "<html><body><h1>Unknown Error</h1></body></html>";
		statusCode = 500;
		response.setStatusCode(statusCode, "Internal Server Error");
	}

	response.setHeader("Content-Type", "text/html");
	response.setBody(fileContents);
}
#include "UploadHandler.hpp"
#include "Server.hpp"

UploadHandler::UploadHandler()
{
}

UploadHandler::UploadHandler(const std::string &webRoot) : _webRoot(webRoot) {};

UploadHandler::~UploadHandler()
{
}

#include <filesystem> // Include the header for std::filesystem

// Function to handle file creation based on HTTP request data
void createFile(HTTPRequest &request)
{
	// Directory to store uploaded files
	const std::string uploadDir = "upload/";

	// Retrieve the list of files from the HTTP request
	std::vector<File> files = request.getFiles();
	std::vector<File>::iterator it;

	// First loop to validate that each file has a "filename" header
	for (it = files.begin(); it != files.end(); ++it)
	{
		// Check if the "filename" header is missing
		if (it->headers.find("filename") == it->headers.end())
		{
			std::cout << "422 Unprocessable Entity (Error: file does not have a name)" << std::endl;
			return;
		}
	}

	// Second loop to create files using the information from the headers
	for (it = files.begin(); it != files.end(); ++it)
	{
		// Construct the full path by prepending the upload directory
		std::string filePath = uploadDir + (it->headers.find("filename"))->second;

		// Open a file with the name specified in the "filename" header under the upload directory
		std::ofstream outfile(filePath.c_str());
		if (outfile.is_open())
		{
			// Write the file content from the request into the newly created file
			outfile << it->fileContent;
			// Close the file to flush the buffer and release the file handle
			outfile.close();
			std::cout << "File created successfully at " << filePath << std::endl;
		}
		else
		{
			// If the file couldn't be opened or created, output an appropriate error message
			std::cout << "422 Unprocessable Entity (Error creating a file at " << filePath << ")" << std::endl;
		}
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

// Possible codes:
// 413 Payload Too Large, 415 Unsupported Media, 401 Unauthorized,
// 403 Forbidden, 500 Internal Server Error
void UploadHandler::handleResponse(HTTPResponse &response, const std::string &code)
{
	std::string fileContents;
	int statusCode;
	std::string statusDescription;

	std::cout << "       Response code: " << code << std::endl;

	// ADD A MESSAGE WITH THE NAME OF THE FILE CREATED

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
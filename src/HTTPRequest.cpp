#include "HTTPRequest.hpp"
#include "webserv.hpp"

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::HTTPRequest(const HTTPRequest &obj)
{
	_method = obj._method;
	_requestTarget = obj._requestTarget;
	_protocolVersion = obj._protocolVersion;
	_queryString = obj._queryString;
	_headers = obj._headers;
	_body = obj._body;
	_uploadBoundary = obj._uploadBoundary;
	_files = obj._files;
	_path = obj._path;
	_cookies = obj._cookies;
}

HTTPRequest &HTTPRequest::operator=(const HTTPRequest &obj)
{
	if (this == &obj)
		return (*this);
	_method = obj._method;
	_requestTarget = obj._requestTarget;
	_protocolVersion = obj._protocolVersion;
	_queryString = obj._queryString;
	_headers = obj._headers;
	_body = obj._body;
	_uploadBoundary = obj._uploadBoundary;
	_files = obj._files;
	_path = obj._path;
	_cookies = obj._cookies;
	return (*this);
}

HTTPRequest::~HTTPRequest()
{
}

std::string HTTPRequest::getMethod() const
{
	return (_method);
}

std::string HTTPRequest::getHost() const
{
	std::multimap<std::string, std::string>::const_iterator it = _headers.find("host");
	if (it != _headers.end())
		return it->second;
	return "";
}

size_t HTTPRequest::getContentLength() const
{
	std::multimap<std::string, std::string>::const_iterator it = _headers.find("content-length");
	if (it != _headers.end())
		return strToInt(it->second);
	return 0;
}

std::string HTTPRequest::getRequestTarget() const
{
	return (_requestTarget);
}

std::string HTTPRequest::getProtocolVersion() const
{
	return (_protocolVersion);
}

std::multimap<std::string, std::string> HTTPRequest::getQueryString() const
{
	return (_queryString);
}

std::multimap<std::string, std::string> HTTPRequest::getHeaders() const
{
	return (_headers);
}

std::pair<std::string, std::string> HTTPRequest::getSingleHeader(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _headers.begin(); it != _headers.end(); ++it)
	{
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

std::string HTTPRequest::getBody() const
{
	return (_body);
}

std::string HTTPRequest::getUploadBoundary() const
{
	return (_uploadBoundary);
}

std::vector<File> HTTPRequest::getFiles() const
{
	return (_files);
}

std::string HTTPRequest::getPath() const
{
	return (_path);
}

std::string HTTPRequest::getRoot() const
{
	return (_root);
}

std::map<std::string, std::string> HTTPRequest::getCookies() const
{
	return (_cookies);
}


void HTTPRequest::setMethod(std::string method)
{
	_method = method;
}

void HTTPRequest::setRequestTarget(std::string requestTarget)
{
	_requestTarget = requestTarget;
}

void HTTPRequest::setQueryString(const std::string &key, const std::string &value)
{
	_queryString.insert(std::make_pair(key, value));
}

// This makes the key lowercase and then inserts the original key
void HTTPRequest::setHeaders(const std::string &key, const std::string &value)
{
	std::string lowerKey = key;
	for (size_t i = 0; i < lowerKey.size(); ++i)
		lowerKey[i] = std::tolower(static_cast<unsigned char>(lowerKey[i]));
	_headers.insert(std::make_pair(lowerKey, value));
}

void HTTPRequest::setBody(const std::string &body)
{
	_body = body;
}

void HTTPRequest::setProtocolVersion(std::string protocolVersion)
{
	_protocolVersion = protocolVersion;
}

void HTTPRequest::setUploadBoundary(const std::string &boundary)
{
	_uploadBoundary = boundary;
}

void HTTPRequest::setFiles(struct File &file)
{
	_files.push_back(file);
}

void HTTPRequest::setFileContent(const std::string &content)
{
	_files.back().fileContent = content;
}

void HTTPRequest::setPath(const std::string path)
{
	this->_path = path;
}

void HTTPRequest::setRoot(const std::string root)
{
	this->_root = root;
}

void HTTPRequest::setCookies(const std::string &key, const std::string &value)
{
	_cookies.insert(std::make_pair(key, value));
}

std::ostream &operator<<(std::ostream &out, const HTTPRequest &obj)
{
	std::multimap<std::string, std::string> headers = obj.getHeaders();
	std::multimap<std::string, std::string> queryString = obj.getQueryString();
	std::string body = obj.getBody();
	std::vector<File> files = obj.getFiles();

	std::multimap<std::string, std::string>::iterator it;
	out << "---------------------Variables--------------------" << std::endl;
	for (it = queryString.begin(); it != queryString.end(); it++)
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	out << "---------------------End--------------------------" << std::endl;

	out << "---------------------Headers----------------------" << std::endl;
	for (it = headers.begin(); it != headers.end(); it++)
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
	out << "---------------------End--------------------------" << std::endl;

	out << "---------------------Body-------------------------" << std::endl;
	// std::cout << body << std::endl;
	out << "---------------------End--------------------------" << std::endl;

	out << "---------------------File-Upload------------------" << std::endl;

	for (std::vector<File>::iterator it = files.begin(); it != files.end(); ++it)
	{
		for (std::map<std::string, std::string>::iterator it2 = it->headers.begin(); it2 != it->headers.end(); ++it2)
			std::cout << "Key: " << it2->first << ", Value: " << it2->second << std::endl;
		std::cout << "Data: " << it->fileContent << std::endl;
	}
	out << "---------------------End--------------------------" << std::endl;

	std::cout << "---------------------Cookies----------------------" << std::endl;
	std::map<std::string, std::string> cookies = obj.getCookies();
	for (std::map<std::string, std::string>::iterator it = cookies.begin(); it != cookies.end(); ++it)
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	std::cout << "---------------------End--------------------------" << std::endl;
	return (out);
}

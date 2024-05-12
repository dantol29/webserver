#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>
#include "webserv.hpp"

class HTTPRequest
{
  public:
	HTTPRequest();
	~HTTPRequest();
	HTTPRequest(const HTTPRequest &obj);
	HTTPRequest &operator=(const HTTPRequest &obj);

	// GETTERS
	std::string getMethod() const;
	std::string getRequestTarget() const;
	std::string getProtocolVersion() const;
	std::string getHost() const;
	std::string getUploadBoundary() const;
	size_t getContentLength() const;
	std::multimap<std::string, std::string> getQueryString() const;
	std::multimap<std::string, std::string> getHeaders() const;
	std::pair<std::string, std::string> getSingleHeader(std::string key) const;
	std::string getBody() const;
	std::vector<struct File> getFiles() const;
	std::string getPath() const;
	std::string getRoot() const;

	// SETTERS
	void setMethod(std::string method);
	void setRequestTarget(std::string requestTarget);
	void setProtocolVersion(std::string protocolVersion);
	void setQueryString(const std::string &key, const std::string &value);
	void setHeaders(const std::string &key, const std::string &value);
	void setBody(const std::string &body);
	void setUploadBoundary(const std::string &boundary);
	void setFiles(struct File &file);
	void setFileContent(const std::string &content);
	void setPath(const std::string &path);
	void setRoot(const std::string &root);

  private:
	// VARIABLES
	std::string _method;
	std::string _requestTarget;
	std::string _protocolVersion;
	std::multimap<std::string, std::string> _queryString;
	std::multimap<std::string, std::string> _headers;
	std::string _body;
	std::string _uploadBoundary;
	std::vector<File> _files;
	std::string _path;
	std::string _root;
};

std::ostream &operator<<(std::ostream &out, const HTTPRequest &a);

#endif
#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <vector>

class HTTPRequest
{
  public:
	HTTPRequest();
	~HTTPRequest();

	// GETTERS
	std::string getMethod() const;
	std::string getHost() const;
	std::string getRequestTarget() const;
	std::string getProtocolVersion() const;
	std::multimap<std::string, std::string> getQueryString() const;
	std::multimap<std::string, std::string> getHeaders() const;
	std::pair<std::string, std::string> getSingleHeader(std::string key) const;
	std::vector<std::string> getBody() const;
	// SETTERS
	void setMethod(std::string method);
	void setRequestTarget(std::string requestTarget);
	void setProtocolVersion(std::string protocolVersion);
	void setQueryString(const std::string &key, const std::string &value);
	void setHeaders(const std::string &key, const std::string &value);
	void setBody(const std::string &body);

  private:
	HTTPRequest(const HTTPRequest &obj);
	HTTPRequest &operator=(const HTTPRequest &obj);

	// VARIABLES
	std::string _method;
	std::string _requestTarget;
	std::string _protocolVersion;
	std::multimap<std::string, std::string> _queryString;
	std::multimap<std::string, std::string> _headers;
	std::vector<std::string> _body;
};

std::ostream &operator<<(std::ostream &out, const HTTPRequest &a);

#endif
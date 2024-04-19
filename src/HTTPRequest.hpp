#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

// TODO: remove this later
#define MAX_URI 200

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

	// UTILS
	bool saveVariables(std::string &variables);
	void makeHeadersLowCase();
	bool isValidHost(std::string host);
	bool isValidContentType(std::string type);
	bool isOrigForm(std::string &requestTarget, int &queryStart);
	void skipRequestLine(const char *request, unsigned int &i);
	void skipHeader(const char *request, unsigned int &i);
	bool hasMandatoryHeaders(HTTPRequest &obj);
	std::string extractValue(std::string &variables, int &i);
	std::string extractKey(std::string &variables, int &i, int startPos);
	std::string extractRequestTarget(const char *request, unsigned int &i);
	std::string extractVariables(std::string &requestTarget, bool &isOriginForm);
	std::string extractProtocolVersion(const char *request, unsigned int &i);
	std::string extractMethod(const char *request, unsigned int &i);
	std::string extractHeaderKey(const char *request, unsigned int &i);
	std::string extractHeaderValue(const char *request, unsigned int &i);
	unsigned int extractLineLength(const char *request, unsigned int &i);
	std::string extractLine(const char *request, unsigned int &i, const unsigned int &size);
};

std::ostream &operator<<(std::ostream &out, const HTTPRequest &a);

#endif
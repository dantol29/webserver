#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

// #include "webserv.hpp"
#include "HTTPRequest.hpp"
#include "webserv.hpp"
#include <iostream>
#include <sys/wait.h>
#include <string.h>
#include <utility>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

class Environment
{
  private:
	std::map<std::string, std::string> metaVars;

  public:
	Environment();									  // Default constructor if needed
	Environment(const Environment &other);			  // Copy constructor
	Environment &operator=(const Environment &other); // Copy assignment operator
													  // access unique var
	void setVar(const std::string &key, const std::string &value);
	std::string getVar(const std::string &key) const;
	void printMetaVars() const;

	// transform request to meta vars (Environment object)
	bool isAuthorityForm(const HTTPRequest &request);
	void RequestTargetToMetaVars(HTTPRequest request, Environment &env);
	std::string formatQueryString(const std::multimap<std::string, std::string> &queryParams) const;
	std::pair<std::string, std::string> separatePathAndInfo(const std::string &requestTarget) const;
	void subtractQueryFromPathInfo(std::string& pathInfo, const std::string& queryString);
	void HTTPRequestToMetaVars(HTTPRequest request, Environment &env);

	// convert to execve format
	std::vector<char *> getForExecve() const;
	~Environment();
};

#endif
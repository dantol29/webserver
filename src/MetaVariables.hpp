#ifndef METAVARIABLES_HPP
#define METAVARIABLES_HPP

#include "webserv.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <sys/wait.h>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

class MetaVariables
{
  private:
	std::map<std::string, std::string> metaVars;

	friend std::ostream &operator<<(std::ostream &out, const MetaVariables &instancePrinted);

  public:
	MetaVariables();
	MetaVariables(const MetaVariables &other);
	MetaVariables &operator=(const MetaVariables &other);
	// access unique var
	void setVar(const std::string &key, const std::string &value);
	std::string getVar(const std::string &key) const;

	// transform request to meta vars (MetaVariables object)
	// bool isAuthorityForm(const HTTPRequest &request);
	// void RequestTargetToMetaVars(HTTPRequest request, MetaVariables &env);
	std::string formatQueryString(const std::multimap<std::string, std::string> &queryParams) const;
	std::pair<std::string, std::string> separatePathAndInfo(const std::string &requestTarget) const;
	void subtractQueryFromPathInfo(std::string &pathInfo, const std::string &queryString);
	void HTTPRequestToMetaVars(const HTTPRequest &request, MetaVariables &env);

	// convert to execve format
	std::vector<char *> getForExecve() const;
	~MetaVariables();
};

#endif
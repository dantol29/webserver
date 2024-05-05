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
	void setVar(const std::string &key, const std::string &value);
	std::string getVar(const std::string &key) const;

	std::string formatQueryString(const std::multimap<std::string, std::string> &queryParams) const;
	std::pair<std::string, std::string> separatePathAndInfo(const std::string &requestTarget) const;
	void subtractQueryFromPathInfo(std::string &pathInfo, const std::string &queryString);
	void HTTPRequestToMetaVars(const HTTPRequest &request, MetaVariables &env);
	std::vector<std::string> getForExecve() const;
	~MetaVariables();
};

#endif
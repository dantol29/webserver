#include "webserv.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <sys/wait.h>
#include <string.h>
#include <utility>
#include <map>
#include <vector>

class Environment {
private:
    std::map<std::string, std::string> envVars;

public:
    Environment(); // Default constructor if needed
    Environment(const Environment& other); // Copy constructor
    Environment& operator=(const Environment& other); // Copy assignment operator
	// access unique var
    void setVar(const std::string& key, const std::string& value);
    std::string getVar(const std::string& key) const;

	// transform request to meta vars (Environment object)
	bool isAuthorityForm(const HTTPRequest& request);
	void RequestTargetToMetaVars(HTTPRequest request, Environment& env);
	void HTTPRequestToMetaVars(char* rawRequest, Environment& env);

	// convert to execve format
    std::vector<char*> getForExecve() const;
    ~Environment();
};
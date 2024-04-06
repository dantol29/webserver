#include "webserv.hpp"
#include <string.h>
/**
 * @brief Sets or updates an environment variable.
 * 
 * This method allows setting a new environment variable or updating the value
 * of an existing one in the internal environment variable storage.
 * 
 * @param key The name of the environment variable.
 * @param value The value to be assigned to the environment variable.
 */
void Environment::setVar(const std::string& key, const std::string& value) {
	envVars[key] = value;
}

/**
 * @brief Generates a vector of C-style strings suitable for execve.
 * 
 * This method converts the stored environment variables into a format that can
 * be used with the execve system call. Each string in the returned vector is
 * in the form of "key=value". The memory for these strings is dynamically
 * allocated and must be freed by the caller. The list is terminated with a NULL
 * pointer as required by execve.
 * 
 * @return A vector of C-style strings representing the environment variables,
 *         suitable for passing to execve.
 */
std::vector<char*> Environment::getForExecve() const {
	std::vector<char*> result;
	for (std::map<std::string, std::string>::const_iterator it = envVars.begin(); it != envVars.end(); ++it) {
		std::string env = it->first + "=" + it->second;
		char* envCStr = new char[env.size() + 1];
		ft_strcpy(envCStr, env.c_str());
		result.push_back(envCStr);
	}
	result.push_back(NULL);
	return result;
}

Environment::~Environment() {
	std::vector<char*> envp = getForExecve();
	for (size_t i = 0; i < envp.size(); ++i) {
		delete[] envp[i];
	}
}

//definition and implementation counts for capitalization of file names (PascalCase)
//git didn't let me push just after changing the case of the name of the file, which is why I comment here
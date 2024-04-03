#include <map>
#include <vector>
#include <string>

class Environment {
private:
// std::map stores elements in a sorted order based on the key.
// Each element is a pair consisting of a key and a corresponding value.
	std::map<std::string, std::string> envVars;

public:
	void setVar(const std::string& key, const std::string& value);

	std::vector<char*> getForExecve() const;

	~Environment();
};
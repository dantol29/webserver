#include <map>
#include <vector>
#include <string>

class Environment {
private:
    std::map<std::string, std::string> envVars;

public:
    void setVar(const std::string& key, const std::string& value);
    std::string getVar(const std::string& key) const;
    std::vector<char*> getForExecve() const;
    ~Environment();
};
#include <map>
#include <vector>
#include <string>

class Environment {
private:
    std::map<std::string, std::string> envVars;

public:
    Environment(); // Default constructor if needed
    Environment(const Environment& other); // Copy constructor
    Environment& operator=(const Environment& other); // Copy assignment operator
    void setVar(const std::string& key, const std::string& value);
    std::string getVar(const std::string& key) const;
    std::vector<char*> getForExecve() const;
    ~Environment();
};
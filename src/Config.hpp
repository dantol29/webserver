#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include "ServerBlock.hpp"

class Config
{
  public:
	Config();
	~Config();
	Config(const Config &obj);
	Config &operator=(const Config &obj);
	std::vector<ServerBlock> getServerBlocks() const;
	std::string getErrorMessage() const;

	void parse(const char *file); // main method
  private:
	std::vector<ServerBlock> _server;

	bool error(std::string message);
	bool parseFile(const char *file);
	bool parseLocation(std::string &line, std::ifstream &config);
	bool saveVariable(const std::string &line);
	bool saveLocationVariable(const std::string &line, std::string &key, std::string &value);
	bool isLocation(const std::string &line);
	bool checkVariablesKey();
	bool checkVariablesValue(std::map<std::string, std::string> var);
	bool checkErrorPage(std::map<std::string, std::string> list);
	bool pathExists(std::map<std::string, std::string> list, std::string variable);

	// internal variables for parsing
	std::string _errorMessage;
	std::string _tmpPath;
	ServerBlock _tmpServer;
};

std::ostream &operator<<(std::ostream &out, const Config &fixed);

#endif
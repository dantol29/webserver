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
	bool parse(std::ifstream &config); // main method

	private:
		std::vector<ServerBlock> _serverBlocks;
		bool		error(std::string message);
		bool		parseLocation(std::string& line, std::ifstream& config);
		bool		saveDirective(const std::string& line);
		bool		saveLocationDirective(const std::string& line, std::string& key, std::string& value);
		bool		isLocation(const std::string& line);
		bool		pathExists(std::map<std::string, std::string> list, std::string variable);

	// internal variables for parsing
	std::string _errorMessage;
	std::string _tmpPath;
	ServerBlock _tmpServerBlock;
};

std::ostream &operator<<(std::ostream &out, const Config &fixed);

#endif
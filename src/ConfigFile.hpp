#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <map>
#include <string>
#include <vector>

class ConfigFile
{
	public:
		ConfigFile(char *file);
		~ConfigFile(){};
		std::string	getErrorMessage() const;
		std::map<std::string, std::string> getVariables() const;
	private:
		ConfigFile();
		ConfigFile(const ConfigFile& obj);
		ConfigFile& operator=(const ConfigFile& obj);
		bool		error(std::string message, char *line);
		bool		parseFile(char *file);
		bool		parseLine(char *line, int mode);
		bool		parseLocation(char *line, int fd);
		bool		isLocation(char *line);
		std::map<std::string, std::string> _variables;
		std::vector<std::string> _locations;
		std::string _errorMessage;
};

#endif
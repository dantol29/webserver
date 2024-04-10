#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <string>

class ConfigFile
{
	public:
		ConfigFile(char *file);
		~ConfigFile(){};
		std::string	getErrorMessage() const;
	private:
		ConfigFile();
		ConfigFile(const ConfigFile& obj);
		ConfigFile& operator=(const ConfigFile& obj);
		bool		error(std::string message, char *line);
		bool		parseFile(char *file);
		bool		parseLine(char *line, std::pair<std::string, std::string>& var, std::string key);
		std::pair<std::string, std::string> _listen;
		std::pair<std::string, std::string> _host;
		std::pair<std::string, std::string> _serverName;
		std::pair<std::string, std::string> _errorPage;
		std::pair<std::string, std::string> _clientMaxBodySize;
		std::pair<std::string, std::string> _root;
		std::pair<std::string, std::string> _index;
		std::string _errorMessage;
};

#endif
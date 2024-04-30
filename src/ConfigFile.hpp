#ifndef CONFIGFILE_HPP
# define CONFIGFILE_HPP

#include <map>
#include <string>
#include <vector>

class ConfigFile
{
	public:
		ConfigFile();
		~ConfigFile(){};
		void parse(char *file);
		std::string	getErrorMessage() const;
		std::map<std::string, std::string> getVariables() const;
		std::pair<std::string, std::string> getVariables(std::string key) const;
		std::vector<std::map<std::string, std::string> > getLocations() const;
	private:
		ConfigFile(const ConfigFile& obj);
		ConfigFile& operator=(const ConfigFile& obj);
		void		createDefaultFile();
		bool		error(std::string message, char *line);
		bool		parseFile(char *file);
		bool		parseLocation(char *line, int fd);
		bool		saveVariable(char *line);
		bool		saveLocationVariable(char *line, std::string& key, std::string& value);
		bool		isLocation(char *line);
		bool		checkVariablesKey();
		bool		checkVariablesValue(std::map<std::string, std::string> var);
		bool		checkErrorPage(std::map<std::string, std::string> list);
		bool		pathExists(std::map<std::string, std::string> list, std::string variable);
		std::map<std::string, std::string> _variables;
		std::vector<std::map<std::string, std::string> > _locations;
		std::string _errorMessage;
		std::string	_tmpPath;
};

std::ostream& operator<<(std::ostream& out, const ConfigFile& fixed);

#endif
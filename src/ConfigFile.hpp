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
		std::string _errorMessage;
};

#endif
#include "Config.hpp"
#include "webserv.hpp"
#include <unistd.h> // access
#include <dirent.h> // opendir
#include <fstream>

Config::Config(const char *file)
{
	std::ifstream configFile;

	if (file)
		configFile.open(file);
	else
		configFile.open(CONFIG_FILE_DEFAULT_PATH);
	
	try {
		parse(configFile);
	}
	catch (const char* error){
		_errorMessage = error;
	}
	if (!_errorMessage.empty())
		std::cerr << _errorMessage << std::endl;
}

Config::Config()
{
}

Config::~Config()
{
}

Config::Config(const Config &obj)
{
	*this = obj;
}

Config &Config::operator=(const Config &obj)
{
	if (this == &obj)
		return (*this);
	_serverBlocks = obj._serverBlocks;
	_errorMessage = obj._errorMessage;
	return (*this);
}

std::vector<ServerBlock> Config::getServerBlocks() const
{
	return (_serverBlocks);
}

std::string Config::getErrorMessage() const
{
	return (_errorMessage);
}

bool Config::setError(std::string message)
{
	_errorMessage = message;
	return (false);
}

// [TAB][KEY][SP][VALUE][;]
bool Config::saveDirective(const std::string &line)
{
	std::string key;
	std::string value;
	unsigned int i = 0;
	int start;

	if (line[i++] != '\t') // [TAB]
		return (false);
	start = i;
	while (i < line.length() && line[i] != ' ')
		i++;
	key = line.substr(start, i - start); // [KEY]

	if (line[i++] != ' ' || line[i] == ' ') // [SP]
		return (false);
	start = i;
	while (i < line.length() && line[i] != ';')
		i++;
	value = line.substr(start, i - start); // [VALUE]

	if (line[i++] != ';' || i < line.length()) // [;]
		return (false);

	return (_tmpServerBlock.addDirective(key, value, false));
}

// [TAB][LOCATION][SP][/PATH][SP][{]
bool Config::isLocation(const std::string &line)
{
	unsigned int i = 0;
	int start;

	if (line[i++] != '\t') // [TAB]
		return (false);

	start = i;
	while (i < line.length() && line[i] != ' ')
		i++;
	if (line.substr(start, i) != "location ") // [LOCATION]
		return (false);

	if (line[i++] != ' ') // [SP]
		return (false);

	start = i;
	while (i < line.length() && line[i] != ' ')
		i++;
	_tmpPath = line.substr(start, i - start); // [PATH]
	if (_tmpPath.empty())
		return (false);

	if (line[i++] != ' ') // [SP]
		return (false);

	if (line[i] != '{' || i + 1 < line.length()) // [{]
		return (false);

	return (true);
}

// [TAB][TAB][KEY][SP][VALUE][;]
bool Config::saveLocationDirective(const std::string &line, std::string &key, std::string &value)
{
	unsigned int i = 0;
	int start;

	if (line[i++] != '\t' || line[i++] != '\t') // [TAB][TAB]
		return (false);

	start = i;
	while (i < line.length() && line[i] != ' ')
		i++;
	key = line.substr(start, i - start); // [KEY]

	if (line[i++] != ' ' || line[i] == ' ') // [SP]
		return (false);

	start = i;
	while (i < line.length() && line[i] != ';')
		i++;
	value = line.substr(start, i - start); // [VALUE]

	if (line[i] != ';' || i + 1 < line.length()) // [;]
		return (false);

	return (true);
}

bool	Config::parseLocation(std::string& line, std::ifstream& config)
{
	std::string	key;
	std::string	value;

	_tmpServerBlock.addDirective("path", _tmpPath, true);
	while (std::getline(config, line))
	{
		if (line == "\t}")
			break;
		if (!saveLocationDirective(line, key, value))
			return (setError("Config file: Syntax error"));
		if (!_tmpServerBlock.addDirective(key, value, true))
			return (false);
	}
	return (true);
}

bool Config::parse(std::ifstream &config)
{
	std::string line;
	if (!config.is_open())
		return (setError("Config file: Invalid file"));

	while (!config.eof())
	{
		while (std::getline(config, line) && line.empty())
			; // skip empty lines

		if (config.eof()) // if file end reached
			break;

		if (line != "server {") // start of the server block
			return (setError("Config file: Syntax error ( no server { )"));

		while (std::getline(config, line))
		{
			if (line.empty()) // empty lines are allowed
				continue;
			if (line == "}") // end of server blcok
			{
				_serverBlocks.push_back(_tmpServerBlock);
				break;
			}
			if (isLocation(line)) // start of location block
				parseLocation(line, config);
			else if (!saveDirective(line)) // variables outside of location
				return (setError("Config file: Syntax error (invalid var in the root)"));
		}
		_tmpServerBlock.deleteData(); // delete saved data
	}
	if (_serverBlocks.size() < 1)
		return (setError("Config file: No valid server blocks"));
	return (true);
}

bool Config::pathExists(std::map<std::string, std::string> list, std::string variable)
{
	std::map<std::string, std::string>::iterator it;
	unsigned int start = 0;

	it = list.find(variable);
	if (it != list.end())
	{
		for (unsigned int i = 0; i < it->second.length(); ++i)
		{
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			DIR *dir = opendir((it->second.substr(start, i - start)).c_str());
			if (!dir)
				return (setError(("Config file: Invalid " + variable).c_str()));
			closedir(dir);
			if (isVulnerablePath(it->second.substr(start, i - start)))
				return (setError("Config file: Path is vulnerable"));
		}
	}
	return (true);
}

std::ostream &operator<<(std::ostream &out, const Config &a)
{
	if (!a.getErrorMessage().empty())
	{
		out << a.getErrorMessage();
		return (out);
	}

	std::vector<ServerBlock> server = a.getServerBlocks();

	for (std::vector<ServerBlock>::iterator it = server.begin(); it != server.end(); ++it)
	{
		Directives var = it->getDirectives();
		std::vector<Directives> loc = it->getLocations();

		std::cout << "------------------Server-Block------------------------" << std::endl;
		for (unsigned int i = 0; i < var._listen.size(); ++i)
			std::cout << var._listen[i] << " ";
		std::cout << "server_name: ";
		for (unsigned int i = 0; i < var._serverName.size(); ++i)
			std::cout << var._serverName[i] << " ";
		for (unsigned int i = 0; i < var._errorPage.size(); ++i)
			std::cout << var._errorPage[i].first << " " << var._errorPage[i].second << std::endl;
		std::cout << "index: ";
		for (unsigned int i = 0; i < var._index.size(); ++i)
			std::cout << var._index[i] << " ";
		std::cout << "root: " << var._root << std::endl;
		std::cout << "client_max_body_size: " << var._clientMaxBodySize << std::endl;
		std::cout << "autoindex: " << var._autoindex << std::endl;
		std::cout << "allowed_methods: ";
		for (unsigned int i = 0; i < var._allowedMethods.size(); ++i)
			std::cout << var._allowedMethods[i] << " ";
		std::cout << "alias: " << var._alias << std::endl;

		for (unsigned int i = 0; i < loc.size(); ++i)
		{
			std::cout << "------------------Location-Block------------------------" << std::endl;
			std::cout << "path: " << loc[i]._path << std::endl;
			for (unsigned int i = 0; i < loc[i]._listen.size(); ++i)
				std::cout << loc[i]._listen[i] << " ";
			std::cout << "server_name: ";
			for (unsigned int i = 0; i < loc[i]._serverName.size(); ++i)
				std::cout << loc[i]._serverName[i] << " ";
			for (unsigned int i = 0; i < loc[i]._errorPage.size(); ++i)
				std::cout << loc[i]._errorPage[i].first << " " << loc[i]._errorPage[i].second << std::endl;
			for (unsigned int i = 0; i < loc[i]._index.size(); ++i)
				std::cout << loc[i]._index[i] << " ";
			std::cout << "root: " << loc[i]._root << std::endl;
			std::cout << "client_max_body_size: " << loc[i]._clientMaxBodySize << std::endl;
			std::cout << "autoindex: " << loc[i]._autoindex << std::endl;
			std::cout << "allowed_methods: ";
			for (unsigned int i = 0; i < loc[i]._allowedMethods.size(); ++i)
				std::cout << loc[i]._allowedMethods[i] << " ";
			std::cout << "alias: " << loc[i]._alias << std::endl;
		}
		std::cout << "------------------END---------------------------------" << std::endl;
	}

	return (out);
}
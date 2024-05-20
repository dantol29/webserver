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
	_fileName = (file) ? file : CONFIG_FILE_DEFAULT_PATH;

	try
	{
		parse(configFile);
	}
	catch (const char *error)
	{
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

std::string Config::getFileName() const
{
	return (_fileName);
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

bool Config::parseLocation(std::string &line, std::ifstream &config)
{
	std::string key;
	std::string value;

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

std::ostream &operator<<(std::ostream &out, const Config &file)
{
	if (!file.getErrorMessage().empty())
	{
		out << file.getErrorMessage();
		return (out);
	}

	std::vector<ServerBlock> server = file.getServerBlocks();
	int serverBlockCounter = 0;

	for (std::vector<ServerBlock>::iterator it = server.begin(); it != server.end(); ++it)
	{
		Directives var = it->getDirectives();
		std::vector<Directives> loc = it->getLocations();
		serverBlockCounter++;
		std::cout << "Config file: " << file.getFileName() << std::endl;

		std::cout << "------------------Server-Block #" << serverBlockCounter << "----------------------" << std::endl;

		for (unsigned int i = 0; i < var._listen.size(); ++i)
		{
			std::cout << "ip: " << var._listen[i].getIp() << std::endl;
			std::cout << "port: " << var._listen[i].getPort() << std::endl;
			std::cout << "isIpv6: " << (var._listen[i].getIsIpv6() ? "true" : "false") << std::endl;
		}
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
		std::cout << std::endl;
		std::cout << "alias: " << var._alias << std::endl;

		for (unsigned int j = 0; j < loc.size(); ++j)
		{
			std::cout << "------------------Location-Block------------------------" << std::endl;
			std::cout << "path: " << loc[j]._path << std::endl;
			for (unsigned int k = 0; k < loc[j]._listen.size(); ++k)
			{
				std::cout << "ip: " << loc[j]._listen[k].getIp() << std::endl;
				std::cout << "port: " << loc[j]._listen[k].getPort() << std::endl;
				std::cout << "isIpv6: " << loc[j]._listen[k].getIsIpv6() << std::endl;
			}
			std::cout << "server_name: ";
			for (unsigned int l = 0; l < loc[j]._serverName.size(); ++l)
				std::cout << loc[j]._serverName[l] << " ";
			for (unsigned int m = 0; m < loc[j]._errorPage.size(); ++m)
				std::cout << loc[j]._errorPage[m].first << " " << loc[j]._errorPage[m].second << std::endl;
			for (unsigned int n = 0; n < loc[j]._index.size(); ++n)
				std::cout << loc[j]._index[n] << " ";
			std::cout << "root: " << loc[j]._root << std::endl;
			std::cout << "client_max_body_size: " << loc[j]._clientMaxBodySize << std::endl;
			std::cout << "autoindex: " << loc[j]._autoindex << std::endl;
			std::cout << "allowed_methods: ";
			for (unsigned int o = 0; o < loc[j]._allowedMethods.size(); ++o)
				std::cout << loc[j]._allowedMethods[o] << " ";
			std::cout << std::endl;
			std::cout << "alias: " << loc[j]._alias << std::endl;
		}
		std::cout << "------------------END---------------------------------" << std::endl;
	}

	return (out);
}

#include "ConfigFile.hpp"
#include <iostream>
#include <cstring>

int	checkFile(const char *path);
char	*get_next_line(int fd);

ConfigFile::ConfigFile(const ConfigFile& obj){
	*this = obj;
}

ConfigFile& ConfigFile::operator=(const ConfigFile& obj){
	if (this != &obj)
		*this = obj;
	return (*this);
}

std::string	ConfigFile::getErrorMessage() const{
	return (_errorMessage);
}

std::map<std::string, std::string> ConfigFile::getVariables() const{
	return (_variables);
}

std::pair<std::string, std::string> ConfigFile::getVariables(std::string key) const{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _variables.begin(); it != _variables.end(); ++it){
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

std::vector<std::map<std::string, std::string> > ConfigFile::getLocations() const{
	return (_locations);
}

bool	ConfigFile::error(std::string message, char *line){
	if (line != NULL)
		delete line;
	_errorMessage = message;
	return (false);
}


// [TAB][KEY][SP][VALUE][;]
bool	ConfigFile::saveVariable(char *line){
	std::string stringLine(line);
	std::string	key;
	std::string value;
	int i = 0;
	int	start;

	if (line[i++] != '\t') // [TAB]
		return (false);
	start = i;
	while (line[i] && line[i] != ' ')
		i++;
	key = stringLine.substr(start, i - start); // [KEY]
	if (line[i++] != ' ' || line[i] == ' ') // [SP]
		return (false);
	start = i;
	while (line[i] && line[i] != ';')
		i++;
	value = stringLine.substr(start, i - start); // [VALUE]
	if (line[i++] != ';') // [;]
		return (false);
	if (line[i] != '\n' || line[i + 1] != '\0')
		return (false);
	_variables.insert(std::make_pair(key, value));
	if (line != NULL)
		delete line;
	line = NULL;
	return (true);
}

// [TAB][LOCATION][SP][/PATH][SP][{]
bool	ConfigFile::isLocation(char *line){
	std::string stringLine(line);
	std::string	path;
	int i = 0;
	int start;
	if (line[i++] != '\t') // [TAB]
		return (false);
	start = i;
	while (line[i] && line[i] != ' ')
		i++;
	if (stringLine.substr(start, i) != "location ") // [LOCATION]
		return (false);
	if (line[i++] != ' ') // [SP]
		return (false);
	start = i;
	while (line[i] && line[i] != ' ')
		i++;
	_tmpPath = stringLine.substr(start, i - start); // [PATH]
	if (_tmpPath.empty())
		return (false);
	if (line[i++] != ' ') // [SP]
		return (false);
	if (line[i] != '{') // [{]
		return (false);
	//_locations.insert();
	return (true);
}

// [TAB][TAB][KEY][SP][VALUE][;]
bool	ConfigFile::saveLocationVariable(char *line, std::string& key, std::string& value){
	std::string stringLine(line);
	int i = 0;
	int	start;

	if (line[i++] != '\t' || line[i++] != '\t') // [TAB][TAB]
		return (false);
	start = i;
	while (line[i] && line[i] != ' ')
		i++;
	key = stringLine.substr(start, i - start); // [KEY]
	if (line[i++] != ' ' || line[i] == ' ') // [SP]
		return (false);
	start = i;
	while (line[i] && line[i] != ';')
		i++;
	value = stringLine.substr(start, i - start); // [VALUE]
	if (line[i++] != ';') // [;]
		return (false);
	if (line[i] != '\n' || line[i + 1] != '\0')
		return (false);
	if (line != NULL)
		delete line;
	line = NULL;
	return (true);
}

bool	ConfigFile::parseLocation(char *line, int fd){
	std::map<std::string, std::string> var;
	std::string	key;
	std::string	value;

	var.insert(std::make_pair("path", _tmpPath));
	while (1){
		line = get_next_line(fd);
		if (line == NULL || std::strcmp(line, "\t}\n") == 0)
			break ;
		if (!saveLocationVariable(line, key, value))
			return (error("Config file: Syntax error", line));
		var.insert(std::make_pair(key, value));
	}
	_locations.push_back(var);
	return (true);
}

bool	ConfigFile::parseFile(char *file){
	char	*line;
	int fd = checkFile(file);
	if (fd == -1) { return (error("Config file: Invalid file", NULL)); }

	line = get_next_line(fd);
	if (line == NULL || std::strcmp(line, "server {\n") != 0) { return (error("Config file: Syntax error", line)); }
	delete line;

	while (1){
		line = get_next_line(fd);
		if (line == NULL) { break ; }
		else if (std::strcmp(line, "}") == 0 && !get_next_line(fd)) { delete line; return (true); }
		else if (std::strcmp(line, "\n") == 0) { delete line; }
		else if (isLocation(line)) { parseLocation(line, fd); }
		else if (!saveVariable(line)) { return (error("Config file: Syntax error", line)); }
	}
	return (error("Config file: Syntax error", line));
}

ConfigFile::ConfigFile(char *file) : _errorMessage(""), _tmpPath(""){
	parseFile(file);
	//checkVariables
}
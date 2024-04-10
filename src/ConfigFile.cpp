#include "ConfigFile.hpp"
#include <iostream>
#include <cstring>

int	checkFile(const char *path);
char	*get_next_line(int fd);

std::string	ConfigFile::getErrorMessage() const{
	return (_errorMessage);
}

std::map<std::string, std::string> ConfigFile::getVariables() const{
	return (_variables);
}

bool	ConfigFile::error(std::string message, char *line){
	if (line != NULL)
		delete line;
	_errorMessage = message;
	return (false);
}


// mode 0: [TAB][KEY][SP][VALUE][;]
// mode 1: [TAB][TAB][KEY][SP][VALUE][;]
bool	ConfigFile::parseLine(char *line, int mode){
	std::string stringLine(line);
	std::string	key;
	std::string value;
	int i = 0;
	int	start;

	if (mode == 1){
		if (line[i++] != '\t' || line[i++] != '\t') // [TAB][TAB]
			return (false);
	} else {
		if (line[i++] != '\t')
			return (false);
	}
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
	if (line[i] != ';') // [;]
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
	path = stringLine.substr(start, i - 1); // [PATH]
	if (line[i++] != ' ') // [SP]
		return (false);
	if (line[i] != '{') // [{]
		return (false);
	_locations.push_back(path);
	return (true);
}

bool	ConfigFile::parseLocation(char *line, int fd){
	while (1){
		std::cout << "aaaaa" << std::endl;
		line = get_next_line(fd);
		if (!parseLine(line, 1))
			return (error("Config file: Syntax error", line));	
	}
	return (true);
}

bool	ConfigFile::parseFile(char *file){
	char	*line;
	int fd = checkFile(file);
	if (fd == -1)
		return (error("Config file: Invalid file", NULL));

	line = get_next_line(fd);
	if (std::strcmp(line, "server {\n") != 0)
		return (error("Config file: Syntax error", line));
	delete line;

	while (1){
		line = get_next_line(fd);
		if (line == NULL)
			return (false);
		if (isLocation(line))
			parseLocation(line, fd);
		else if (!parseLine(line, 0))
			return (error("Config file: Syntax error", line));
	}
	return (true);
}

ConfigFile::ConfigFile(char *file) : _errorMessage(""){
	parseFile(file);
	//checkVariables
}
#include "ConfigFile.hpp"
#include <iostream>
#include <cstring>

int	checkFile(const char *path);
char	*get_next_line(int fd);

std::string	ConfigFile::getErrorMessage() const{
	return (_errorMessage);
}

bool	ConfigFile::error(std::string message, char *line){
	if (line != NULL)
		delete line;
	_errorMessage = message;
	return (false);
}


// [TAB][KEY][SP][VALUE][;]
bool	ConfigFile::parseLine(char *line, std::pair<std::string, std::string>& var, std::string key){
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
	if (line[i] != ';') // [;]
		return (false);
	var = std::make_pair(key, value);
	delete line;
	line = NULL;
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

	line = get_next_line(fd);
	if (!parseLine(line, _listen, "listen"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _host, "host"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _serverName, "server_name"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _errorPage, "error_page"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _clientMaxBodySize, "client_max_body_size"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _root, "root"))
		return (error("Config file: Syntax error", line));
	line = get_next_line(fd);
	if (!parseLine(line, _index, "index"))
		return (error("Config file: Syntax error", line));
	return (true);
}
	// std::cout << _listeningPort.first << ", " << _listeningPort.second << std::endl;

ConfigFile::ConfigFile(char *file) : _errorMessage(""){
	parseFile(file);
}
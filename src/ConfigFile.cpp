#include "ConfigFile.hpp"
#include <iostream>
int	checkFile(const char *path);
char	*get_next_line(int fd);

std::string	ConfigFile::getErrorMessage() const{
	return (_errorMessage);
}

ConfigFile::ConfigFile(char *file) : _errorMessage(""){
	char	*line;
	int fd = checkFile(file);
	if (fd == -1){
		_errorMessage = "Invalid file or path";
		return ;
	}
	while (1){
		line = get_next_line(fd);
		if (!line)
			break ;
		std::cout << line << std::endl;
	}
}
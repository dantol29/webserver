#include "ConfigFile.hpp"
#include <iostream>
#include <cstring>
#include <list>
#include <algorithm>
#include "webserv.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>

bool	isVulnerablePath(const std::string& path);
bool	isValidErrorCode(std::string errorCode);

ConfigFile::ConfigFile() : _errorMessage(""), _tmpPath("")
{
}

ConfigFile::ConfigFile(const ConfigFile& obj)
{
	*this = obj;
}

ConfigFile& ConfigFile::operator=(const ConfigFile& obj)
{
	if (this != &obj)
		*this = obj;
	return (*this);
}

std::string	ConfigFile::getErrorMessage() const
{
	return (_errorMessage);
}

std::map<std::string, std::string> ConfigFile::getVariables() const
{
	return (_variables);
}

std::pair<std::string, std::string> ConfigFile::getVariables(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _variables.begin(); it != _variables.end(); ++it){
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

std::vector<std::map<std::string, std::string> > ConfigFile::getLocations() const
{
	return (_locations);
}

bool	ConfigFile::error(std::string message)
{
	_errorMessage = message;
	return (false);
}

// [TAB][KEY][SP][VALUE][;]
bool	ConfigFile::saveVariable(const std::string& line)
{
	std::string	key;
	std::string value;
	unsigned int i = 0;
	int	start;

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
	// TODO: line[i + 1] != '\0'

	_variables.insert(std::make_pair(key, value));
	return (true);
}

// [TAB][LOCATION][SP][/PATH][SP][{]
bool	ConfigFile::isLocation(const std::string& line)
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

	if (line[i] != '{' || i + 1 < line.length() ) // [{]
		return (false);

	return (true);
}

// [TAB][TAB][KEY][SP][VALUE][;]
bool	ConfigFile::saveLocationVariable(const std::string& line, std::string& key, std::string& value)
{
	unsigned int i = 0;
	int	start;

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

	// TODO: line[i + 1] != '\0'
	return (true);
}

bool	ConfigFile::parseLocation(std::string& line, std::ifstream& config){
	std::map<std::string, std::string> var;
	std::string	key;
	std::string	value;

	var.insert(std::make_pair("path", _tmpPath));
	while (std::getline(config, line)){
		if (line == "\t}")
			break ;
		if (!saveLocationVariable(line, key, value))
			return (error("Config file: Syntax error"));
		var.insert(std::make_pair(key, value));
	}
	_locations.push_back(var);
	return (true);
}

bool	ConfigFile::parseFile(const char *file)
{
	std::string line;
	std::ifstream config(file);
	
	if (!config.is_open())
		return (error("Config file: Invalid file"));

	std::getline(config, line);
	if (line != "server {")
		return (error("Config file: Syntax error ( no server { )"));

	while (std::getline(config, line))
	{
		if (line.empty())	
			continue;
		if (line == "}")
			return (true);
		if (isLocation(line))
			parseLocation(line, config);
		else if (!saveVariable(line))
			return (error("Config file: Syntax error (invalid var in the root)"));
	}
	return (error("Config file: Syntax error (server is not enclosed)"));
}

bool	ConfigFile::checkVariablesKey(){
	std::string var[] = {"listen", "host", "server_name", "error_page", \
	"index", "root", "client_max_body_size", "autoindex", "allow_methods", \
	"alias", "cgi_path", "cgi_ext"};
	std::list<std::string> validVar(var, var + sizeof(var) / sizeof(var[0]));

	for (std::map<std::string, std::string>::iterator it = _variables.begin(); it != _variables.end(); ++it){
		if (std::find(validVar.begin(), validVar.end(), it->first) == validVar.end())
			return (error("Config file: Invalid variable"));
	}
	for (unsigned int i = 0; i < _locations.size(); ++i){
		for (std::map<std::string, std::string>::iterator it = _locations[i].begin(); it != _locations[i].end(); ++it){
			if (it->first == "path")
				continue;
			if (std::find(validVar.begin(), validVar.end(), it->first) == validVar.end())
				return (error("Config file: Invalid variable(in the location)"));
		}
	}
	return (true);
}

bool	ConfigFile::pathExists(std::map<std::string, std::string> list, std::string variable)
{
	std::map<std::string, std::string>::iterator it;
	unsigned int	start = 0;

	it = list.find(variable);
	if (it != list.end()){
		for (unsigned int i = 0; i < it->second.length(); ++i){
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			DIR* dir = opendir((it->second.substr(start, i - start)).c_str());
			if (!dir)
				return (error(("Config file: Invalid " + variable).c_str()));
			closedir(dir);
			if (isVulnerablePath(it->second.substr(start, i - start)))
				return (error("Config file: Path is vulnerable"));
		}
	}
	return (true);
}

bool	ConfigFile::checkErrorPage(std::map<std::string, std::string> list)
{
	// count1 and count2 to check if both path and number are present
	int				count1 = 0;
	int				count2 = 0;
	unsigned int	start = 0;
	std::map<std::string, std::string>::iterator it;

	it = list.find("error_page");
	if (it != list.end()){
		for (unsigned int i = 0; i < it->second.length(); ++i){
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			if (access((it->second.substr(start, i - start)).c_str(), F_OK) == 0){
				if (isVulnerablePath(it->second.substr(start, i - start)))
					return (error("Config file: Path is vulnerable"));
				count1++;
				continue ;
			}
			if (!isValidErrorCode(it->second.substr(start, i - start)))
				return (error("Config file: Invalid error_page"));
			count2++;
		}
		if (count1 == 0 || count1 > 1 || count2 == 0)
			return (error("Config file: Invalid error_page"));
	}
	return (true);
}


bool	ConfigFile::checkVariablesValue(std::map<std::string, std::string> var)
{
	std::string tmp_meth[] = {"GET", "POST", "DELETE"};
	std::string tmp_cgi[] = {".py", ".php", ".pl", ".cgi"};
	std::list<std::string> methods(tmp_meth, tmp_meth + sizeof(tmp_meth) / sizeof(tmp_meth[0]));
	std::list<std::string> cgi_ext(tmp_cgi, tmp_cgi + sizeof(tmp_cgi) / sizeof(tmp_cgi[0]));
	std::map<std::string, std::string>::iterator it;
	unsigned int	start = 0;

	// // ROOT
	// if (!pathExists(var, "root"))
	// 	return (false);
	// ALIAS
	if (!pathExists(var, "alias"))
		return (false);
	// CGI_PATH
	if (!pathExists(var, "cgi_path"))
		return (false);
	// ERROR_PAGE
	if (!checkErrorPage(var))
		return (false);
	// ALLOW_METHODS
	it = var.find("allow_methods");
	if (it != var.end()){
		for (unsigned int i = 0; i < it->second.length(); ++i){
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			if (std::find(methods.begin(), methods.end(), it->second.substr(start, i - start)) == methods.end())
				return (error("Config file: Invalid allow_method"));
		}
	}
	// AUTOINDEX
	it = var.find("autoindex");
	if (it != var.end())
		if (it->second != "on")
			return (error("Config file: Invalid autoindex"));
	// CLIENT_MAX_BODY_SIZE
	it = var.find("client_max_body_size");
	if (it != var.end()){
		if (!isNumber(it->second))
			return (error("Config file: Invalid client_max_body_size"));
	}
	// CGI_EXT
	it = var.find("cgi_ext");
	if (it != var.end()){
		for (unsigned int i = 0; i < it->second.length(); ++i){
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			if (std::find(cgi_ext.begin(), cgi_ext.end(), it->second.substr(start, i - start)) == cgi_ext.end())
				return (error("Config file: Invalid cgi_ext"));
		}
	}
	// INDEX
	it = var.find("index");
	if (it != var.end()){
		for (unsigned int i = 0; i < it->second.length(); ++i){
			start = i;
			while (i < it->second.length() && it->second[i] != ' ')
				i++;
			if (access((it->second.substr(start, i - start)).c_str(), F_OK) == 0){
				if (isVulnerablePath(it->second.substr(start, i - start)))
					return (error("Config file: Path is vulnerable"));
				return (true);
			}
		}
		return (error("Config file: Invalid index"));
	}
	return (true);
}

void ConfigFile::parse(const char *file)
{
	parseFile(file);
	checkVariablesKey();
	checkVariablesValue(_variables);
	// check each location variables values
	for (unsigned int i = 0; i < _locations.size(); ++i)
		checkVariablesValue(_locations[i]);
}

std::ostream& operator<<(std::ostream& out, const ConfigFile& a){
	if (!a.getErrorMessage().empty()){
		out << a.getErrorMessage();
		return (out);
	}
	std::map<std::string, std::string> var = a.getVariables();
	std::vector<std::map<std::string, std::string> > loc = a.getLocations();

	for (std::map<std::string, std::string>::iterator it = var.begin(); it != var.end(); ++it)
		out << "Key: " << it->first << ", Value: " << it->second << std::endl;
    for (unsigned int i = 0; i < loc.size(); ++i){
		for (std::map<std::string, std::string>::iterator it = loc[i].begin(); it != loc[i].end(); it++)
			out << i << "LKey: " << it->first << ":" << ", LValue: " << it->second << ":"<< std::endl;
	}
	return (out);
}
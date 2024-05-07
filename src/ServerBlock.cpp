#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{
}

ServerBlock::~ServerBlock()
{
}

ServerBlock::ServerBlock(const ServerBlock &obj)
{
	_locations = obj._locations;
	_directives = obj._directives;
}

ServerBlock &ServerBlock::operator=(const ServerBlock &obj)
{
	if (this != &obj)
	{
		_locations = obj._locations;
		_directives = obj._directives;
	}
	return (*this);
}


bool ServerBlock::addDirective(std::string key, std::string& value, bool isLocation)
{
	std::string var[] = {"listen", "server_name", "error_page", \
	"index", "root", "client_max_body_size", "autoindex", "allow_methods", \
	"alias", "path"};
	std::list<std::string> validVar(var, var + sizeof(var) / sizeof(var[0]));

	if (std::find(validVar.begin(), validVar.end(), key) == validVar.end())
	{
		std::cout << "Unknown key: "<< key << std::endl;
		return (false);
	}

	if (key == "listen")
		setListen(value, isLocation);
	else if (key == "server_name")
		setServerName(transformServerName(value), isLocation);
	else if (key == "error_page")
		setErrorPage(transformErrorPage(value), isLocation);
	else if (key == "index")
		setIndex(transformIndex(value), isLocation);
	else if (key == "root")
		setRoot(value, isLocation);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(value, isLocation);
	else if (key == "autoindex")
		setAutoIndex(value, isLocation);
	else if (key == "allow_methods")
		setAllowedMethods(transformAllowedMethods(value), isLocation);
	else if (key == "alias")
		setAlias(value, isLocation);
	else if (key == "path" && isLocation)
		setLocationPath(value);

	return (true);
}

void ServerBlock::deleteData()
{
	_locations.clear();
	_directives._listen.clear();
	_directives._serverName.clear();
	_directives._errorPage = std::make_pair(0, "");
	_directives._index.clear();
	_directives._root.clear();
	_directives._clientMaxBodySize = 0;
	_directives._autoindex = false;
	_directives._allowedMethods.clear();
	_directives._alias.clear();
	_directives._path.clear();
}

Directives ServerBlock::getDirectives() const
{
	return (_directives);
}

std::vector<Directives> ServerBlock::getLocations() const
{
	return (_locations);
}

std::string ServerBlock::getListen() const
{
	return (_directives._listen);
}

std::vector<std::string> ServerBlock::getServerName() const
{
	return (_directives._serverName);
}

std::pair<int, std::string> ServerBlock::getErrorPage() const
{
	return (_directives._errorPage);
}

std::vector<std::string> ServerBlock::getIndex() const
{
	return (_directives._index);
}

std::string ServerBlock::getRoot() const
{
	return (_directives._root);
}

size_t ServerBlock::getClientMaxBodySize() const
{
	return (_directives._clientMaxBodySize);
}

bool ServerBlock::getAutoIndex() const
{
	return (_directives._autoindex);
}

std::vector<std::string> ServerBlock::getAllowedMethods() const
{
	return (_directives._allowedMethods);
}

std::string ServerBlock::getAlias() const
{
	return (_directives._alias);
}

void ServerBlock::setListen(std::string& str, bool isLocation)
{
	if (!isLocation)
		_directives._listen = str;
	else
		_locations.back()._listen = str;
}

void ServerBlock::setServerName(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
		_directives._serverName = str;
	else
		_locations.back()._serverName = str;
}

void ServerBlock::setErrorPage(std::pair<int, std::string> str, bool isLocation)
{
	if (!isLocation)
		_directives._errorPage = str;
	else
		_locations.back()._errorPage = str;
}

void ServerBlock::setIndex(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
		_directives._index = str;	
	else
		_locations.back()._index = str;
}

void ServerBlock::setRoot(std::string& str, bool isLocation)
{
	if (!isLocation)
		_directives._root = str;
	else
		_locations.back()._root = str;
}

void ServerBlock::setClientMaxBodySize(std::string& str, bool isLocation)
{
	if (strToInt(str) == -1)
		throw ("Invalid client_max_body_size");

	size_t n = strToInt(str);
	
	if (!isLocation)
		_directives._clientMaxBodySize = n;	
	else
		_locations.back()._clientMaxBodySize = n;
}

void ServerBlock::setAutoIndex(std::string& str, bool isLocation)
{
	bool a;

	if (str == "on")
		a = true;
	else if (str == "off")
		a = false;
	else
		throw("Invalid autoindex");

	if (!isLocation)
		_directives._autoindex = a;
	else
		_locations.back()._autoindex = a;
}

void ServerBlock::setAllowedMethods(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
		_directives._allowedMethods = str;	
	else
		_locations.back()._allowedMethods = str;
}

void ServerBlock::setAlias(std::string& str, bool isLocation)
{
	if (!isLocation)
		_directives._alias = str;	
	else
		_locations.back()._alias = str;
}

void ServerBlock::setLocationPath(std::string str)
{
	// create a new location block (element in _locations vector)
	_locations.push_back(Directives());
	_locations.back()._path = str;
}

std::vector<std::string> ServerBlock::transformServerName(std::string& str)
{
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	return (newStr);
}

std::pair<int, std::string> ServerBlock::transformErrorPage(std::string& str)
{
	std::string path;
	int error;

	int index = str.find(' ');
	error = strToInt(str.substr(0, index));
	if (!isValidErrorCode(str.substr(0, index)))
		throw ("Invalid error code");
	path = str.substr(index + 1);
	return (std::make_pair(error, path));
}

std::vector<std::string> ServerBlock::transformIndex(std::string& str)
{
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	return (newStr);
}

std::vector<std::string> ServerBlock::transformAllowedMethods(std::string& str)
{
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	
	for (unsigned int i = 0; i < newStr.size(); ++i)
	{
		if (newStr[i] != "GET" && newStr[i] != "POST" \
		&& newStr[i] != "PUT" && newStr[i] != "DELETE")
			throw ("Invalid method");
	}
	return (newStr);
}
#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{
}

ServerBlock::~ServerBlock()
{
}

ServerBlock::ServerBlock(const ServerBlock& obj)
{
	_locations = obj._locations;
	_variables = obj._variables;
}

ServerBlock& ServerBlock::operator=(const ServerBlock& obj)
{
	if (this != &obj)
	{
		_locations = obj._locations;
		_variables = obj._variables;
	}
	return (*this);
}


bool ServerBlock::addVariable(std::string key, std::string& value, bool isLocation)
{
	std::string var[] = {"listen", "server_name", "error_page", \
	"index", "root", "client_max_body_size", "autoindex", "allow_methods", \
	"alias", "path"};
	std::list<std::string> validVar(var, var + sizeof(var) / sizeof(var[0]));

	if (std::find(validVar.begin(), validVar.end(), key) == validVar.end())
		return (false);

	if (key == "listen")
		setListen(value, isLocation);
	else if (key == "server_name")
		setServerName(value, isLocation);
	else if (key == "error_page")
		setErrorPage(value, isLocation);
	else if (key == "index")
		setIndex(value, isLocation);
	else if (key == "root")
		setRoot(value, isLocation);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(value, isLocation);
	else if (key == "autoindex")
		setAutoIndex(value, isLocation);
	else if (key == "allow_methods")
		setAllowedMethods(value, isLocation);
	else if (key == "alias")
		setAlias(value, isLocation);
	else if (key == "path" && isLocation)
		setLocationPath(value);

	return (true);
}

void ServerBlock::deleteData()
{
	_locations.clear();
	_variables._listen.clear();
	_variables._serverName.clear();
	_variables._errorPage.clear();
	_variables._index.clear();
	_variables._root.clear();
	_variables._clientMaxBodySize = 0;
	_variables._autoindex = false;
	_variables._allowedMethods.clear();
	_variables._alias.clear();
	_variables._path.clear();
}

Variables ServerBlock::getVariables() const
{
	return (_variables);
}

std::vector<Variables> ServerBlock::getLocations() const
{
	return (_locations);
}

std::string ServerBlock::getListen() const
{
	return (_variables._listen);
}

std::vector<std::string> ServerBlock::getServerName() const
{
	return (_variables._serverName);
}

std::vector<int, std::string> ServerBlock::getErrorPage() const
{
	return (_variables._errorPage);
}

std::vector<std::string> ServerBlock::getIndex() const
{
	return (_variables._index);
}

std::string ServerBlock::getRoot() const
{
	return (_variables._root);
}

size_t ServerBlock::getClientMaxBodySize() const
{
	return (_variables._clientMaxBodySize);
}

bool ServerBlock::getAutoIndex() const
{
	return (_variables._autoindex);
}

std::vector<std::string> ServerBlock::getAllowedMethods() const
{
	return (_variables._allowedMethods);
}

std::string ServerBlock::getAlias() const
{
	return (_variables._alias);
}

void ServerBlock::setListen(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._listen = str;
	else
		_locations.back()._listen = str;
}

void ServerBlock::setServerName(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._serverName = str;
	else
		_locations.back()._serverName = str;
}

void ServerBlock::setErrorPage(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._errorPage = str;
	else
		_locations.back()._errorPage = str;
}

void ServerBlock::setIndex(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._index = str;	
	else
		_locations.back()._index = str;
}

void ServerBlock::setRoot(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._root = str;
	else
		_locations.back()._root = str;
}

void ServerBlock::setClientMaxBodySize(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._clientMaxBodySize = n;	
	else
		_locations.back()._clientMaxBodySize = n;
}

void ServerBlock::setAutoIndex(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._autoindex = a;
	else
		_locations.back()._autoindex = a;
}

void ServerBlock::setAllowedMethods(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._allowedMethods = str;	
	else
		_locations.back()._allowedMethods = str;
}

void ServerBlock::setAlias(std::string& str, bool isLocation)
{
	if (!isLocation)
		_variables._alias = str;	
	else
		_locations.back()._alias = str;
}

void ServerBlock::setLocationPath(std::string str)
{
	// create a new location block (element in _locations vector)
	_locations.push_back(Variables());
	_locations.back()._path = str;
}
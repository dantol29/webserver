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
	"alias", "path", "cgi_path", "cgi_ext"};
	std::list<std::string> validVar(var, var + sizeof(var) / sizeof(var[0]));

	if (std::find(validVar.begin(), validVar.end(), key) == validVar.end())
	{
		std::cout << "Unknown key: "<< key << std::endl;
		return (false);
	}

	if (key == "listen")
		setListen(transformServerListen(value), isLocation);
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
	else if (key == "cgi_path")
		setCgiPath(value, isLocation);
	else if (key == "cgi_ext")
		setCgiExt(transformCgiExt(value), isLocation);
	else if (key == "path" && isLocation)
		setLocationPath(value);

	return (true);
}

void ServerBlock::deleteData()
{
	_locations.clear();
	_directives._listen._ip.clear();
	_directives._listen._port.clear();
	_directives._serverName.clear();
	_directives._errorPage.clear();
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

Listen ServerBlock::getListen() const
{
	return (_directives._listen);
}

std::vector<std::string> ServerBlock::getServerName() const
{
	return (_directives._serverName);
}

std::vector<std::pair<int, std::string> > ServerBlock::getErrorPage() const
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

std::string ServerBlock::getCgiPath() const
{
	return (_directives._cgiPath);
}

std::vector<std::string> ServerBlock::getCgiExt() const
{
	return (_directives._cgiExt);
}

void ServerBlock::setListen(Listen str, bool isLocation)
{
	if (!isLocation){
		for (unsigned int i = 0; i < str._port.size(); ++i){
			_directives._listen._port.push_back(str._port[i]);
		}
		_directives._listen._ip = str._ip;
	}
	else
		throw ("listen directive not allowed in location block");

	for (unsigned int i = 0; i < _directives._listen._port.size(); ++i)
	{
		for (unsigned int j = 0; j < _directives._listen._port.size(); ++j)
		{
			if (i != j && _directives._listen._port[i] == _directives._listen._port[j])
				throw ("Duplicate listen directive");
		}
	}	
}

void ServerBlock::setServerName(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._serverName.size() > 0)
			throw ("server_name already set");
		_directives._serverName = str;
	}
	else
	{
		if (_locations.back()._serverName.size() > 0)
			throw ("server_name already set");
		_locations.back()._serverName = str;
	}
}

void ServerBlock::setErrorPage(std::pair<int, std::string> str, bool isLocation)
{
	if (!isLocation)
		_directives._errorPage.push_back(str);
	else
		_locations.back()._errorPage.push_back(str);
}

void ServerBlock::setIndex(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._index.size() > 0)
			throw ("index already set");
		_directives._index = str;	
	}
	else
	{
		if (_locations.back()._index.size() > 0)
			throw ("index already set");
		_locations.back()._index = str;
	}
}

void ServerBlock::setRoot(std::string& str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._root.size() > 0)
			throw ("root already set");
		_directives._root = str;
	}
	else
	{
		if (_locations.back()._root.size() > 0)
			throw ("root already set");
		_locations.back()._root = str;
	}
}

void ServerBlock::setClientMaxBodySize(std::string& str, bool isLocation)
{
	if (strToInt(str) < 1)
		throw ("Invalid client_max_body_size");

	size_t n = strToInt(str);
	
	if (!isLocation)
	{
		if (_directives._clientMaxBodySize > 0)
			throw ("client_max_body_size already set");
		_directives._clientMaxBodySize = n;	
	}
	else
	{
		if (_locations.back()._clientMaxBodySize > 0)
			throw ("client_max_body_size already set");
		_locations.back()._clientMaxBodySize = n;
	}
}

void ServerBlock::setAutoIndex(std::string& str, bool isLocation)
{
	bool a;

	if (str == "on")
		a = true;
	else if (str == "off")
		a = false;
	else
		throw("Invalid autoindex value (on/off)");

	if (!isLocation)
	{
		if (_directives._autoindex)
			throw ("autoindex already set");
		_directives._autoindex = a;
	}
	else
	{
		if (_locations.back()._autoindex)
			throw ("autoindex already set");
		_locations.back()._autoindex = a;
	}
}

void ServerBlock::setAllowedMethods(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._allowedMethods.size() > 0)
			throw ("allowed_methods already set");
		_directives._allowedMethods = str;	
	}
	else
	{
		if (_locations.back()._allowedMethods.size() > 0)
			throw ("allowed_methods already set");
		_locations.back()._allowedMethods = str;
	}
}

void ServerBlock::setAlias(std::string& str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._alias.size() > 0)
			throw ("alias already set");
		_directives._alias = str;	
	}
	else
	{
		if (_locations.back()._alias.size() > 0)
			throw ("alias already set");
		_locations.back()._alias = str;
	}
}

void ServerBlock::setCgiExt(std::vector<std::string> str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._cgiExt.size() > 0)
			throw ("cgi_ext already set");
		_directives._cgiExt = str;	
	}
	else
	{
		if (_locations.back()._cgiExt.size() > 0)
			throw ("cgi_ext already set");
		_locations.back()._cgiExt = str;
	}
}

void ServerBlock::setCgiPath(std::string str, bool isLocation)
{
	if (!isLocation)
	{
		if (_directives._cgiPath.size() > 0)
			throw ("cgi_path already set");
		_directives._cgiPath = str;	
	}
	else
	{
		if (_locations.back()._cgiPath.size() > 0)
			throw ("cgi_path already set");
		_locations.back()._cgiPath = str;
	}
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

void ServerBlock::makeListenStruct(std::vector<std::string> newStr, Listen& listen)
{
	int port;
	std::string ip;
	std::string portStr;
	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP socket
	hints.ai_flags = AI_NUMERICHOST; // Treat node parameter as a numeric address

	for (unsigned int i = 0; i < newStr.size(); ++i)
	{
		if (newStr[i].find(':') != std::string::npos)
		{
			ip = newStr[i].substr(0, newStr[i].find(':'));
			portStr = newStr[i].substr(newStr[i].find(':') + 1);
			port = strToInt(portStr);

			if (port < 1 || port > 65535)
				throw ("Invalid port");
			if (getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0)
				throw ("Invalid IP address");
			
			freeaddrinfo(res);
			listen._port.push_back(port);
			listen._ip = ip;
		}
		else
		{
			port = strToInt(newStr[i]);
			if (port < 1 || port > 65535)
				throw ("Invalid port");
			listen._port.push_back(port);
		}
	}
	if (listen._ip.empty())
		listen._ip = "None";
	if (listen._port.empty())
		listen._port.push_back(0);
}

Listen ServerBlock::transformServerListen(std::string& str)
{
	Listen listen;
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	makeListenStruct(newStr, listen);
	return (listen);
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

std::vector<std::string> ServerBlock::transformCgiExt(std::string& str)
{
	std::string extensions[] = {".php", ".py", ".pl"};
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' ')){
		if (std::find(extensions, extensions + sizeof(extensions) / sizeof(extensions[0]), name) \
		== extensions + sizeof(extensions) / sizeof(extensions[0]))
			throw ("Invalid CGI extension");
		newStr.push_back(name);
	}
	if (newStr.empty()){
		if (std::find(extensions, extensions + sizeof(extensions) / sizeof(extensions[0]), str) \
		== extensions + sizeof(extensions) / sizeof(extensions[0]))
			throw ("Invalid CGI extension");
		newStr.push_back(str);
	}
	return (newStr);
}
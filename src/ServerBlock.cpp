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

bool ServerBlock::addDirective(std::string key, std::string &value, bool isLocation)
{
	std::string var[] = {"listen",
						 "server_name",
						 "error_page",
						 "index",
						 "root",
						 "client_max_body_size",
						 "autoindex",
						 "allow_methods",
						 "alias",
						 "path",
						 "cgi_path",
						 "cgi_ext",
						 "return"};
	std::list<std::string> validVar(var, var + sizeof(var) / sizeof(var[0]));

	if (std::find(validVar.begin(), validVar.end(), key) == validVar.end())
	{
		std::cout << "Unknown key: " << key << std::endl;
		return (false);
	}

	if (key == "listen")
		transformServerListen(value, isLocation);
	else if (key == "server_name")
		_directives.setServerName(transformServerName(value), *this, isLocation);
	else if (key == "error_page")
		_directives.setErrorPage(transformErrorPage(value), *this, isLocation);
	else if (key == "index")
		_directives.setIndex(transformIndex(value), *this, isLocation);
	else if (key == "root")
		_directives.setRoot(value, *this, isLocation);
	else if (key == "client_max_body_size")
		_directives.setClientMaxBodySize(value, *this, isLocation);
	else if (key == "autoindex")
		_directives.setAutoIndex(value, *this, isLocation);
	else if (key == "allow_methods")
		_directives.setAllowedMethods(transformAllowedMethods(value), *this, isLocation);
	else if (key == "alias")
		_directives.setAlias(value, *this, isLocation);
	else if (key == "cgi_path")
		_directives.setCgiPath(value, *this, isLocation);
	else if (key == "cgi_ext")
		_directives.setCgiExt(transformCgiExt(value), *this, isLocation);
	else if (key == "return")
		_directives.setReturn(value, *this, isLocation);
	else if (key == "path" && isLocation)
		setLocationPath(value);

	return (true);
}

void ServerBlock::deleteData()
{
	_locations.clear();
	_directives._listen.clear();
	_directives._serverName.clear();
	_directives._errorPage.clear();
	_directives._index.clear();
	_directives._root.clear();
	_directives._clientMaxBodySize = 0;
	_directives._autoindex = false;
	_directives._allowedMethods.clear();
	_directives._alias.clear();
	_directives._path.clear();
	_directives._cgiPath.clear();
	_directives._cgiExt.clear();
	_directives._return.clear();
}

Directives ServerBlock::getDirectives() const
{
	return (_directives);
}

std::vector<Directives> ServerBlock::getLocations() const
{
	return (_locations);
}

std::vector<Listen> Directives::getListen() const
{
	return (_listen);
}

std::vector<std::string> Directives::getServerName() const
{
	return (_serverName);
}
// clang-format off
std::vector<std::pair<int, std::string> > Directives::getErrorPage() const
// clang-format on
{
	return (_errorPage);
}

std::vector<std::string> Directives::getIndex() const
{
	return (_index);
}

std::string Directives::getRoot() const
{
	return (_root);
}

size_t Directives::getClientMaxBodySize() const
{
	return (_clientMaxBodySize);
}

bool Directives::getAutoIndex() const
{
	return (_autoindex);
}

std::vector<std::string> Directives::getAllowedMethods() const
{
	return (_allowedMethods);
}

std::string Directives::getAlias() const
{
	return (_alias);
}

std::string Directives::getCgiPath() const
{
	return (_cgiPath);
}

std::vector<std::string> Directives::getCgiExt() const
{
	return (_cgiExt);
}

std::string Directives::getReturn() const
{
	return (_return);
}

void Directives::setListenEntry(Listen listenEntry, bool isLocation)
{
	if (!isLocation)
		_listen.push_back(listenEntry);
	else
		throw("listen directive not allowed in location block");

	for (unsigned int i = 0; i < _listen.size(); ++i)
	{
		for (unsigned int j = 0; j < _listen.size(); ++j)
		{
			if (i != j && _listen[i].getPort() == _listen[j].getPort())
				throw("Duplicate listen directive");
		}
	}
}

void Directives::setServerName(std::vector<std::string> str, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_serverName.size() > 0)
			throw("server_name already set");
		_serverName = str;
	}
	else
	{
		if (block.getLocations().back()._serverName.size() > 0)
			throw("server_name already set");
		block.getLocations().back()._serverName = str;
	}
}

void Directives::setErrorPage(std::pair<int, std::string> str, ServerBlock &block, bool isLocation)
{
	if (isLocation)
		throw("error_page directive not allowed in location block");
	_errorPage.push_back(str);
	
	for (unsigned int i = 0; i < _errorPage.size(); ++i)
	{
		// remove slashes at the begginning
		if ( _errorPage[i].second[0] == '/')
			_errorPage[i].second = _errorPage[i].second.substr(1);
		
		for (unsigned int j = 0; j < _errorPage.size(); ++j)
		{
			if (i != j && _errorPage[i].first == _errorPage[j].first)
				throw("Duplicate error_page directive");
		}
	}
}

void Directives::setIndex(std::vector<std::string> stringsVector, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_index.size() > 0)
			throw("index already set");
		_index = stringsVector;
	}
	else
	{
		if (block.getLocations().back()._index.size() > 0)
			throw("index already set");
		block.getLocations().back()._index = stringsVector;
	}
}

void Directives::setRoot(std::string &str, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_root.size() > 0)
			throw("root already set");
		if (str[0] == '/')
			str = str.substr(1);
		_root = str;
	}
	else
	{
		if (block.getLocations().back()._root.size() > 0)
			throw("root already set");
		if (str[0] == '/')
			str = str.substr(1);
		block.getLocations().back()._root = str;
	}
}

void Directives::setClientMaxBodySize(std::string &str, ServerBlock &block, bool isLocation)
{
	std::cout << "str: " << str << std::endl;
	if (strToInt(str) < 1)
		throw("Invalid client_max_body_size");

	size_t n = strToInt(str);

	if (!isLocation)
	{
		if (_clientMaxBodySize > 0)
			throw("client_max_body_size already set");
		_clientMaxBodySize = n;
	}
	else
	{
		if (block.getLocations().back()._clientMaxBodySize > 0)
			throw("client_max_body_size already set");
		block.getLocations().back()._clientMaxBodySize = n;
	}
}

void Directives::setAutoIndex(std::string &str, ServerBlock &block, bool isLocation)
{
	bool autoIndexValue;

	if (str == "on")
		autoIndexValue = true;
	else if (str == "off")
		autoIndexValue = false;
	else
		throw("Invalid autoindex value (on/off)");

	if (!isLocation)
	{
		if (_autoindex)
			throw("autoindex already set");
		_autoindex = autoIndexValue;
	}
	else
	{
		if (block.getLocations().back()._autoindex)
			throw("autoindex already set");
		block.getLocations().back()._autoindex = autoIndexValue;
	}
}

void Directives::setAllowedMethods(std::vector<std::string> str, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_allowedMethods.size() > 0)
			throw("allowed_methods already set");
		_allowedMethods = str;
	}
	else
	{
		if (block.getLocations().back()._allowedMethods.size() > 0)
			throw("allowed_methods already set");
		block.getLocations().back()._allowedMethods = str;
	}
}

void Directives::setAlias(std::string &str, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_alias.size() > 0)
			throw("alias already set");
		_alias = str;
	}
	else
	{
		if (block.getLocations().back()._alias.size() > 0)
			throw("alias already set");
		block.getLocations().back()._alias = str;
	}
}

void Directives::setCgiExt(std::vector<std::string> stringsVector, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_cgiExt.size() > 0)
			throw("cgi_ext already set");
		_cgiExt = stringsVector;
	}
	else
	{
		// if (_locations.back()._cgiExt.size() > 0)
		if (block.getLocations().back()._cgiExt.size() > 0)
			throw("cgi_ext already set");
		// _locations.back()._cgiExt = str;
		block.getLocations().back()._cgiExt = stringsVector;
	}
}

void Directives::setCgiPath(std::string str, ServerBlock &block, bool isLocation)
{
	if (!isLocation)
	{
		if (_cgiPath.size() > 0)
			throw("cgi_path already set");
		_cgiPath = str;
	}
	else
	{
		if (block.getLocations().back()._cgiPath.size() > 0)
			throw("cgi_path already set");
		block.getLocations().back()._cgiPath = str;
	}
}

void Directives::setReturn(std::string str, ServerBlock &block, bool isLocation)
{
	// if there is a space in the string or if there is no http in the string
	if (str.find(" ") != std::string::npos || str.find("http") == std::string::npos)
		throw("Invalid return directive");
	if (!isLocation)
	{
		if (_return.size() > 0)
			throw("return already set");
		_return = str;
	}
	else
	{
		if (block.getLocations().back()._return.size() > 0)
			throw("return already set");
		block.getLocations().back()._return = str;
	}
}

void ServerBlock::setLocationPath(std::string str)
{
	// create a new location block (element in _locations vector)
	_locations.push_back(Directives());
	_locations.back()._path = str;
}

std::vector<std::string> ServerBlock::transformServerName(std::string &str)
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

// Listen ServerBlock::buildListenStruct(std::string &newStr)
// {
// 	Listen listen;
// 	int port;
// 	std::string ip;
// 	std::string portStr;
// 	bool isIpAndPort = false;
// 	struct addrinfo hints;
// 	struct addrinfo *res;

// 	listen.isIpv6 = false;
// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_UNSPEC;	 // IPv4 or IPv6
// 	hints.ai_socktype = SOCK_STREAM; // TCP socket

// 	// if IPv6 is in [ip]:port format
// 	if (newStr[0] == '[')
// 		newStr.erase(0, 1);
// 	if (newStr.find(']') != std::string::npos)
// 		newStr.replace(newStr.find(']'), 1, "");

// 	// (IPv6:port) or (IPv6) or (IPv4) or (port)
// 	if (getaddrinfo(newStr.c_str(), NULL, &hints, &res) == 0)
// 	{
// 		freeaddrinfo(res);
// 		portStr = newStr;

// 		// (IPv6:port)
// 		if (newStr.find_last_of(':') != std::string::npos)
// 		{
// 			portStr = newStr.substr(newStr.find_last_of(':') + 1);
// 			isIpAndPort = true;
// 		}

// 	port = strToInt(portStr);
// 	if (port >= 1 && port <= 65535)
// 	{
// 		listen._port = port;
// 		if (!isIpAndPort)
// 		{
// 			listen._ip = "Any";
// 			return (listen);
// 		}
// 	}
// 	// is incorrect integer
// 	else if ((port < 1 || port > 65535) && port != -1)
// 		throw("Invalid port");

// 	ip = newStr;
// 	// (IPv6:port)
// 	if (isIpAndPort)
// 		ip = newStr.substr(0, newStr.find_last_of(':'));
// 	listen._ip = ip;
// 	listen.isIpv6 = true;
// }
// (IPv4:port)
// else
// {
// 	ip = newStr.substr(0, newStr.find_last_of(':'));
// 	portStr = newStr.substr(newStr.find_last_of(':') + 1);
// 	port = strToInt(portStr);
// 	if (port < 1 || port > 65535)
// 		throw("Invalid port");
// 	listen._ip = ip;

// 	if (getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0)
// 		throw("Invalid ip");
// 	freeaddrinfo(res);
// 	listen._port = port;
// }

// 	if (listen._ip.empty())
// 		listen._ip = "Any";
// 	if (listen._port == 0)
// 		listen._port = 0;

// 	return (listen);
// }

void ServerBlock::transformServerListen(std::string &str, bool isLocation)
{
	if (isLocation)
		throw("listen directive not allowed in location block");

	Listen listen;
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	for (unsigned int i = 0; i < newStr.size(); ++i)
		// _directives.setListenEntry(buildListenStruct(newStr[i]), false);
		_directives.setListenEntry(Listen(newStr[i]), false);
}

std::pair<int, std::string> ServerBlock::transformErrorPage(std::string &str)
{
	std::string path;
	int error;

	int index = str.find(' ');
	error = strToInt(str.substr(0, index));
	if (!isValidErrorCode(str.substr(0, index)))
		throw("Invalid error code");
	path = str.substr(index + 1);
	return (std::make_pair(error, path));
}

std::vector<std::string> ServerBlock::transformIndex(std::string &str)
{
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
		newStr.push_back(name);
	if (newStr.empty())
		newStr.push_back(str);
	// remove slashes at the begginning
	for (unsigned int i = 0; i < newStr.size(); ++i)
	{
		if (newStr[i][0] == '/')
			newStr[i] = newStr[i].substr(1);
	}
	return (newStr);
}

std::vector<std::string> ServerBlock::transformAllowedMethods(std::string &str)
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
		if (newStr[i] != "GET" && newStr[i] != "POST" && newStr[i] != "PUT" && newStr[i] != "DELETE" &&
			newStr[i] != "SALAD")
			throw("Invalid method");
	}
	return (newStr);
}

std::vector<std::string> ServerBlock::transformCgiExt(std::string &str)
{
	// The user can choose its extensions *among* the following ones
	// (for security reasons)
	std::string extensions[] = {".php", ".py", ".pl", ".cgi", ".magicsalad"};
	std::vector<std::string> newStr;
	std::stringstream ss(str);
	std::string name;

	while (std::getline(ss, name, ' '))
	{
		if (std::find(extensions, extensions + sizeof(extensions) / sizeof(extensions[0]), name) ==
			extensions + sizeof(extensions) / sizeof(extensions[0]))
			throw("Invalid CGI extension");
		while (std::getline(ss, name, ' '))
		{
			if (std::find(extensions, extensions + sizeof(extensions) / sizeof(extensions[0]), name) ==
				extensions + sizeof(extensions) / sizeof(extensions[0]))
				throw("Invalid CGI extension");
			newStr.push_back(name);
		}
		if (newStr.empty())
		{
			if (std::find(extensions, extensions + sizeof(extensions) / sizeof(extensions[0]), str) ==
				extensions + sizeof(extensions) / sizeof(extensions[0]))
				throw("Invalid CGI extension");
			newStr.push_back(str);
		}
	}
	return (newStr);
}
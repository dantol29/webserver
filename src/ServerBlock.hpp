#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "webserv.hpp"

// a list of all accepted variables
// ---------------------------------
// 1. listen, 2. server_name, 3. error_page,
// 4. index, 5. root, 6. client_max_body_size, 7. autoindex,
// 8. allow_methods, 9. alias, 10. cgi_path, 11. cgi_ext
// ---------------------------------

class ServerBlock; // forward declaration
struct Listen
{
	std::string _ip;
	int _port;
	bool _isIpv6;

	Listen()
	{
		_ip = "";
		_port = 0;
		_isIpv6 = false;
	}

	Listen(std::string ip, int port, bool ipv6)
	{
		_ip = ip;
		_port = port;
		_isIpv6 = ipv6;
	}

	Listen(std::string str)
	{
		std::string ip;
		int port;
		std::string portStr;
		bool isIpAndPort = false;
		struct addrinfo hints;
		struct addrinfo *res;

		_isIpv6 = false;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;	 // IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

		// If IPv6:port or [ip]:port format
		if (str[0] == '[')
			str.erase(0, 1);
		if (str.find(']') != std::string::npos)
			str.replace(str.find(']'), 1, "");

		// (IPv6:port) or (IPv6) or (IPv4) or (port)
		if (getaddrinfo(str.c_str(), NULL, &hints, &res) == 0)
		{
			freeaddrinfo(res);
			portStr = str;
			// (IPv6:port)
			if (str.find_last_of(':') != std::string::npos)
			{
				portStr = str.substr(str.find_last_of(':') + 1);
				isIpAndPort = true;
			}

			port = strToInt(portStr);
			if (port >= 1 && port <= 65535)
			{
				_port = port;
				if (!isIpAndPort)
				{
					_ip = "Any";
					return;
				}
			}
			// is incorrect integer
			else if ((port < 1 || port > 65535) && port != -1)
				throw("Invalid port number");

			ip = str;
			// (IPv6:port) or (IPv6)
			if (isIpAndPort)
				ip = str.substr(0, str.find_last_of(':'));
			_ip = ip;
			_isIpv6 = true;
		}
		// (IPv4:port)
		else
		{
			ip = str.substr(0, str.find_last_of(':'));
			portStr = str.substr(str.find_last_of(':') + 1);
			port = strToInt(portStr);
			if (port < 1 || port > 65535)
				throw("Invalid port");
			_ip = ip;

			if (getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0)
				throw("Invalid ip");
			freeaddrinfo(res);
			_port = port;
		}
		if (_ip.empty())
			_ip = "Any";
		if (_port == 0)
			_port = 0;

		return;
	}

	Listen(const Listen &obj)
	{
		_ip = obj._ip;
		_port = obj._port;
		_isIpv6 = obj._isIpv6;
	}

	Listen &operator=(const Listen &obj)
	{
		_ip = obj._ip;
		_port = obj._port;
		_isIpv6 = obj._isIpv6;
		return *this;
	}
};

struct Directives
{
	Directives()
	{
		_listen.clear();
		_serverName.clear();
		_errorPage.clear();
		_index.clear();
		_root = "";
		_clientMaxBodySize = 0;
		_autoindex = false;
		_allowedMethods.clear();
		_alias = "";
		_cgiPath = "";
		_cgiExt.clear();
		_path = "";
	}
	std::vector<Listen> _listen;
	std::vector<std::string> _serverName;
	// clang-format off
	std::vector<std::pair<int, std::string> > _errorPage;
	// clang-format on
	std::vector<std::string> _index;
	std::string _root;
	size_t _clientMaxBodySize;
	bool _autoindex;
	std::vector<std::string> _allowedMethods;
	std::string _alias;
	std::vector<std::string> _cgiExt;
	std::string _cgiPath;
	std::string _path; // only for location blocks

	// GETTERS AND SETTERS
	std::vector<Listen> getListen() const;
	std::vector<std::string> getServerName() const;
	// clang-format off
	std::vector<std::pair<int, std::string> > getErrorPage() const;
	// clang-format on
	std::vector<std::string> getIndex() const;
	std::string getRoot() const;
	size_t getClientMaxBodySize() const;
	bool getAutoIndex() const;
	std::vector<std::string> getAllowedMethods() const;
	std::string getAlias() const;
	std::vector<std::string> getCgiExt() const;
	std::string getCgiPath() const;

	void setListenEntry(Listen listenEntry, bool isLocation);
	void setServerName(std::vector<std::string> str, ServerBlock &block, bool isLocation);
	void setErrorPage(std::pair<int, std::string> str, ServerBlock &block, bool isLocation);
	void setIndex(std::vector<std::string> str, ServerBlock &block, bool isLocation);
	void setRoot(std::string &str, ServerBlock &block, bool isLocation);
	void setClientMaxBodySize(std::string &str, ServerBlock &block, bool isLocation);
	void setAutoIndex(std::string &str, ServerBlock &block, bool isLocation);
	void setAllowedMethods(std::vector<std::string> str, ServerBlock &block, bool isLocation);
	void setAlias(std::string &str, ServerBlock &block, bool isLocation);
	void setCgiExt(std::vector<std::string> stringsVector, ServerBlock &block, bool isLocation);
	void setCgiPath(std::string str, ServerBlock &block, bool isLocation);
};

class ServerBlock
{
  public:
	ServerBlock();
	ServerBlock(const ServerBlock &obj);
	ServerBlock &operator=(const ServerBlock &obj);
	~ServerBlock();

	Directives getDirectives() const;
	std::vector<Directives> getLocations() const;

	void setLocationPath(std::string str);
	bool addDirective(std::string key, std::string &value, bool isLocation);

	// clear ServerBlock
	void deleteData();

  private:
	Directives _directives;
	std::vector<Directives> _locations;

	// TRANSFORMERS
	void transformServerListen(std::string &str, bool isLocation);
	std::vector<std::string> transformServerName(std::string &str);
	std::pair<int, std::string> transformErrorPage(std::string &str);
	std::vector<std::string> transformIndex(std::string &str);
	std::vector<std::string> transformAllowedMethods(std::string &str);
	std::vector<std::string> transformCgiExt(std::string &str);

	// Listen buildListenStruct(std::string &newStr);
};

#endif
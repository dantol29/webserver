#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

#include "webserv.hpp"

// a list of all accepted variables
// ---------------------------------
// 1. listen, 2. server_name, 3. error_page,
// 4. index, 5. root, 6. client_max_body_size, 7. autoindex, 
// 8. allow_methods, 9. alias, (cgi_path, cgi_ext) - optional
// ---------------------------------
struct Variables
{
	std::string _listen;
	std::vector<std::string> _serverName;
	std::vector<int, std::string> _errorPage;
	std::vector<std::string> _index;
	std::string _root;
	size_t _clientMaxBodySize;
	bool _autoindex;
	std::vector<std::string> _allowedMethods;
	std::string _alias;
	std::string _path; // only for location blocks
};

class ServerBlock
{
	public:
		ServerBlock();
		ServerBlock(const ServerBlock& obj);
		ServerBlock& operator=(const ServerBlock& obj);
		~ServerBlock();

		bool addVariable(std::string key, std::string& value, bool isLocation);

		// GETTERS
		Variables getVariables() const; // variables outside of locations
		std::vector<Variables> getLocations() const; // location / {} blocks
		std::string getListen() const;
		std::vector<std::string> getServerName() const;
		std::vector<int, std::string> getErrorPage() const;
		std::vector<std::string> getIndex() const;
		std::string getRoot() const;
		size_t getClientMaxBodySize() const;
		bool getAutoIndex() const;
		std::vector<std::string> getAllowedMethods() const;
		std::string getAlias() const;

		// SETTERS
		void setListen(std::string& str, bool isLocation);
		void setServerName(std::string& str, bool isLocation);
		void setErrorPage(std::string& str, bool isLocation);
		void setIndex(std::string& str, bool isLocation);
		void setRoot(std::string& str, bool isLocation);
		void setClientMaxBodySize(std::string& n, bool isLocation);
		void setAutoIndex(std::string& str, bool isLocation);
		void setAllowedMethods(std::string& str, bool isLocation);
		void setAlias(std::string& str, bool isLocation);
		void setLocationPath(std::string str);

		void deleteData();
	private:
		
		Variables _variables;
		std::vector<Variables> _locations;

};

#endif
#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "webserv.hpp"

// a list of all accepted variables
// ---------------------------------
// 1. listen, 2. server_name, 3. error_page,
// 4. index, 5. root, 6. client_max_body_size, 7. autoindex, 
// 8. allow_methods, 9. alias, (cgi_path, cgi_ext) - optional
// ---------------------------------
struct Directives
{
	std::string _listen;
	std::vector<std::string> _serverName;
	std::pair<int, std::string> _errorPage;
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
	ServerBlock(const ServerBlock &obj);
	ServerBlock &operator=(const ServerBlock &obj);
	~ServerBlock();

		bool addVariable(std::string key, std::string& value, bool isLocation);

		// GETTERS
		Directives getVariables() const; // variables outside of locations
		std::vector<Directives> getLocations() const; // location / {} blocks
		std::string getListen() const;
		std::vector<std::string> getServerName() const;
		std::pair<int, std::string> getErrorPage() const;
		std::vector<std::string> getIndex() const;
		std::string getRoot() const;
		size_t getClientMaxBodySize() const;
		bool getAutoIndex() const;
		std::vector<std::string> getAllowedMethods() const;
		std::string getAlias() const;

		// SETTERS
		void setListen(std::string& str, bool isLocation);
		void setServerName(std::vector<std::string> str, bool isLocation);
		void setErrorPage(std::pair<int, std::string> str, bool isLocation);
		void setIndex(std::vector<std::string> str, bool isLocation);
		void setRoot(std::string& str, bool isLocation);
		void setClientMaxBodySize(std::string& n, bool isLocation);
		void setAutoIndex(std::string& str, bool isLocation);
		void setAllowedMethods(std::vector<std::string> str, bool isLocation);
		void setAlias(std::string& str, bool isLocation);
		void setLocationPath(std::string str);
		
		// clear ServerBlock
		void deleteData();
	private:
		Directives _variables;
		std::vector<Directives> _locations;
		
		// TRANSFORMERS
		std::vector<std::string> transformServerName(std::string& str);
		std::pair<int, std::string> transformErrorPage(std::string& str);
		std::vector<std::string> transformIndex(std::string& str);
		std::vector<std::string> transformAllowedMethods(std::string& str);
		

};

#endif
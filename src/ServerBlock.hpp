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
};

class ServerBlock
{
	public:
		ServerBlock();
		ServerBlock(const ServerBlock& obj);
		ServerBlock& operator=(const ServerBlock& obj);
		~ServerBlock();

		// GETTERS
		Variables getVariables() const; // variables outside of locations
		std::vector<Variables> getLocations() const; // location / {} blocks
		
		// SETTERS
		void setListen(std::string& str);
		void setServerName(std::vector<std::string>& str);
		void setErrorPage(std::vector<int, std::string>& str);
		void setIndex(std::vector<std::string>& str);
		void setRoot(std::string& str);
		void setClientMaxBodySize(size_t& n);
		void setAutoIndex(bool& a);
		void setAllowedMethods(std::vector<std::string>& str);
		void setAlias(std::string& str);

		void deleteData();
	private:
		
		Variables _variables;
		std::vector<Variables> _locations;

};

#endif
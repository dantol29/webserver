#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

#include "webserv.hpp"

class ServerBlock
{
	public:
		ServerBlock();
		ServerBlock(const ServerBlock& obj);
		ServerBlock& operator=(const ServerBlock& obj);
		~ServerBlock();

		std::map<std::string, std::string> getVariables() const; // variables outside of locations
		std::pair<std::string, std::string> getVariables(std::string key) const;
		std::vector<std::map<std::string, std::string> > getLocations() const; // location / {} blocks
		
		void addVariable(std::string& key, std::string& value);
		void addLocation(std::map<std::string, std::string>& var);

		void deleteData();
	private:
		
		// a list of all accepted variables
		// ---------------------------------
		// listen, host, server_name, error_page,
		// index, root, client_max_body_size, autoindex, 
		// allow_methods, alias, cgi_path, cgi_ext
		// ---------------------------------
		std::map<std::string, std::string> _variables;
		std::vector<std::map<std::string, std::string> > _locations;

};

#endif
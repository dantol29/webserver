#ifndef SERVERBLOCK_HPP
# define SERVERBLOCK_HPP

#include "webserv.hpp"

class ServerBlock
{
	public:
		ServerBlock();
		~ServerBlock();

		std::map<std::string, std::string> getVariables() const; // variables outside of locations
		std::pair<std::string, std::string> getVariables(std::string key) const;
		std::vector<std::map<std::string, std::string> > getLocations() const; // location / {} blocks
		
		void addVariable(std::string& key, std::string& value);
		void addLocation(std::map<std::string, std::string>& var);
	private:
		ServerBlock(const ServerBlock& obj);
		ServerBlock& operator=(const ServerBlock& obj);

		std::map<std::string, std::string> _variables;
		std::vector<std::map<std::string, std::string> > _locations;

};

#endif
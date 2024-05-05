#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{
}

ServerBlock::~ServerBlock()
{
}

ServerBlock::ServerBlock(const ServerBlock& obj)
{
}

ServerBlock& ServerBlock::operator=(const ServerBlock& obj)
{
}

std::map<std::string, std::string> ServerBlock::getVariables() const
{
	return (_variables);
}

std::pair<std::string, std::string> ServerBlock::getVariables(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _variables.begin(); it != _variables.end(); ++it){
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}

std::vector<std::map<std::string, std::string> > ServerBlock::getLocations() const
{
	return (_locations);
}


void ServerBlock::addVariable(std::string& key, std::string& value)
{
	_variables.insert(std::make_pair(key, value));
}

void ServerBlock::addLocation(std::map<std::string, std::string>& var)
{
	_locations.push_back(var);
}
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
	_variables = obj._variables;
}

ServerBlock &ServerBlock::operator=(const ServerBlock &obj)
{
	if (this != &obj)
	{
		_locations = obj._locations;
		_variables = obj._variables;
	}
	return (*this);
}

std::map<std::string, std::string> ServerBlock::getVariables() const
{
	return (_variables);
}

std::pair<std::string, std::string> ServerBlock::getVariables(std::string key) const
{
	std::multimap<std::string, std::string>::const_iterator it;

	for (it = _variables.begin(); it != _variables.end(); ++it)
	{
		if (it->first == key)
			return (std::make_pair(it->first, it->second));
	}
	return (std::make_pair("", ""));
}
// clang-format off
std::vector<std::map<std::string, std::string> > ServerBlock::getLocations() const
// clang-format on
{
	return (_locations);
}

void ServerBlock::addVariable(std::string &key, std::string &value)
{
	_variables.insert(std::make_pair(key, value));
}

void ServerBlock::addLocation(std::map<std::string, std::string> &var)
{
	_locations.push_back(var);
}

void ServerBlock::deleteData()
{
	_locations.clear();
	_variables.clear();
}
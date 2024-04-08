#include "Server.hpp"
#include <iostream>
#include <fstream>

// Constructor with config file path
Server::Server(const std::string configFilePath) : _configFilePath(configFilePath)
{
	loadDefaultConfig();
	loadConfig();
}

// Destructor
Server::~Server()
{
}

// Default constructor
Server::Server()
{
	loadDefaultConfig();
}

Server Server::getServer()
{
	return *this;
}

void Server::loadDefaultConfig()
{
	_webRoot = "var/www";
}

std::string Server::getWebRoot() const
{
	return _webRoot;
}

void Server::setWebRoot(const std::string &webRoot)
{
	_webRoot = webRoot;
}

std::string Server::getConfigFilePath() const
{
	return _configFilePath;
}

void Server::loadConfig()
{
	// Add logic to load config from file
}

void Server::start()
{
	// Add logic to start the server
}

void Server::stop()
{
	// Add logic to stop the server
}
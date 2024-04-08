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

// Start listening
void Server::startListen()
{
	// int _serverFD; is a member variable
	// struct sockaddr_in _serverAddr; is a member variable
	int addrLen = sizeof(_serverAddr);

	// Creating socket file descriptor
	if ((_serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("In socket");
		// TODO: consider what to do here. Not sure we want to exit the program.
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	// Set SO_REUSEADDR to allow re-binding to the same address and port
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
	{
		perror("setsockopt");
		// TODO: consider what to do here. Not sure we want to exit the program.
		exit(EXIT_FAILURE);
	}
	if (setsockopt(_serverFD, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt SO_REUSEPORT: Protocol not available, continuing without SO_REUSEPORT");
		// Don't exit on failure; it's not critical for basic server functionality
		// Setting SO_REUSEPORT is not supported on all systems and setting it on the same call was causing the server
		// to fail
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = INADDR_ANY;
	_serverAddr.sin_port = htons(_port);
	ft_memset(_serverAddr.sin_zero, '\0', sizeof _serverAddr.sin_zero);

	if (bind(_serverFD, (struct sockaddr *)&_serverAddr, sizeof(_serverAddr)) < 0)
	{
		perror("In bind");
		// TODO: consider what to do here. Not sure we want to exit the program.
		exit(EXIT_FAILURE);
	}
	// 10 is the maximum size of the queue of pending connections: check this value.
	if (listen(_serverFD, 10) < 0)
	{
		perror("In listen");
		// TODO: consider what to do here. Not sure we want to exit the program.
		exit(EXIT_FAILURE);
	}
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
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server
{
  public:
	Server();
	Server(const std::string configFilePath);
	~Server();

	std::string getWebRoot() const;
	void setWebRoot(const std::string &webRoot);
	std::string getConfigFilePath() const;

	Server getServer();
	void start();
	void stop();

  private:
	// Copy constructor
	Server(const Server &other);
	// Assignment operator
	Server &operator=(const Server &other);
	void loadConfig();
	void loadDefaultConfig();

	std::string _configFilePath;
	std::string _webRoot;
};

#endif
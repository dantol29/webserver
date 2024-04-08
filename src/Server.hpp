#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

class Server
{
  public:
	Server();
	Server(const std::string configFilePath);
	~Server();

	void startListen();

	int getPort() const;
	void setPort(int port);

	std::string getWebRoot() const;
	void setWebRoot(const std::string &webRoot);
	std::string getConfigFilePath() const;

	Server getServer();
	void start();
	void stop();

  private:
	int _port;
	int _serverFD;
	std::string _configFilePath;
	std::string _webRoot;
	struct sockaddr_in _serverAddr;
	std::vector<int> _FDs;
	// Copy constructor
	Server(const Server &other);
	// Assignment operator
	Server &operator=(const Server &other);
	void loadConfig();
	void loadDefaultConfig();
};

#endif
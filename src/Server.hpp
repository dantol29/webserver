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
	void startPollEventLoop();

	int getPort() const;
	void setPort(int port);

	std::string getWebRoot() const;
	void setWebRoot(const std::string &webRoot);
	std::string getConfigFilePath() const;

	Server getServer();
	void start();
	void stop();

  private:
	/* Private Attributes*/
	int _port;
	int _serverFD;
	std::string _configFilePath;
	std::string _webRoot;
	struct sockaddr_in _serverAddr;
	std::vector<struct pollfd> _FDs;

	/* Private Methods */

	void loadConfig();
	void loadDefaultConfig();
	void handleConnection(int clientFD);

	/* Not avaiabel constructors */

	// Copy constructor
	Server(const Server &other);
	// Assignment operator
	Server &operator=(const Server &other);
};

#endif
#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include "ServerBlock.hpp"

class ServerSocket
{
  public:
	ServerSocket();
	ServerSocket(int serverFD, Listen listen);
	~ServerSocket();
	ServerSocket(const ServerSocket &obj);
	ServerSocket &operator=(const ServerSocket &obj);
	int getServerFD() const;
	Listen getListen() const;
	struct sockaddr_in6 getServerSocketAddr() const;
	void prepareServerSocketAddr();

  private:
	int _serverFD;
	Listen _listen;
	struct sockaddr_in6 _serverSocketAddr;
};
#endif
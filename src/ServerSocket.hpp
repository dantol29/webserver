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
	struct sockaddr_storage getServerSocketAddr() const;
	void prepareServerSocketAddr();

  private:
	int _serverFD;
	Listen _listen;
	struct sockaddr_storage _serverSocketAddr;
};
#endif
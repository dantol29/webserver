#include "ServerSocket.hpp"
#include <unistd.h>

ServerSocket::ServerSocket()
{
	_serverFD = -1;
	_listen = Listen();
	memset(&_serverSocketAddr, 0, sizeof(_serverSocketAddr));
}

ServerSocket::ServerSocket(int serverFD, Listen listen)
{
	_serverFD = serverFD;
	_listen = listen;
	memset(&_serverSocketAddr, 0, sizeof(_serverSocketAddr));
	if (_listen._isIpv6)
	{
		// c_style case
		// struct sockaddr_in6 *serverSocketAddr = (struct sockaddr_in6 *)&_serverSocketAddr;
		// c++ style case
		struct sockaddr_in6 *serverSocketAddr = reinterpret_cast<struct sockaddr_in6 *>(&_serverSocketAddr);
		socklen_t len = sizeof(*serverSocketAddr);
		getsockname(_serverFD, (struct sockaddr *)serverSocketAddr, &len);
	}
	else
	{
		struct sockaddr_in *serverSocketAddr = reinterpret_cast<struct sockaddr_in *>(&_serverSocketAddr);
		socklen_t len = sizeof(*serverSocketAddr);
		getsockname(_serverFD, (struct sockaddr *)serverSocketAddr, &len);
	}
}

ServerSocket::~ServerSocket()
{
}

ServerSocket::ServerSocket(const ServerSocket &obj)
{
	if (this == &obj)
		return;
	_serverFD = obj._serverFD;
	_listen = obj._listen;
	_serverSocketAddr = obj._serverSocketAddr;
}

ServerSocket &ServerSocket::operator=(const ServerSocket &obj)
{
	_serverFD = obj._serverFD;
	_listen = obj._listen;
	_serverSocketAddr = obj._serverSocketAddr;
	return *this;
}

int ServerSocket::getServerFD() const
{
	return _serverFD;
}

Listen ServerSocket::getListen() const
{
	return _listen;
}

struct sockaddr_storage ServerSocket::getServerSocketAddr() const
{
	return _serverSocketAddr;
}
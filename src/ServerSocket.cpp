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
	// We don't initialize the serverSocketAddr here but in the bindToPort method, just before calling bind()
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

void ServerSocket::prepareServerSocketAddr()
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = _listen._isIpv6 ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::string port = toString(_listen._port);
	char *ip = NULL;
	if (!_listen._ip.empty() && _listen._ip != "any")
		ip = const_cast<char *>(_listen._ip.c_str());
	int status = getaddrinfo(ip, port.c_str(), &hints, &res);
	if (status != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
		return;
	}
	if (res == NULL)
	{
		std::cerr << "getaddrinfo: res is NULL" << std::endl;
		return;
	}
	std::cout << "Before memcpy, port: " << ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port) << std::endl;
	memcpy(&_serverSocketAddr, res->ai_addr, res->ai_addrlen);
	std::cout << "After memcpy, port: " << ntohs(((struct sockaddr_in *)&_serverSocketAddr)->sin_port) << std::endl;
	freeaddrinfo(res);
}
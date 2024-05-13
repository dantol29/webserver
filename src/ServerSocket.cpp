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

struct sockaddr_in6 ServerSocket::getServerSocketAddr() const
{
	return _serverSocketAddr;
}

std::string mapIPv4ToIPv6(const std::string &ipv4Address)
{
	// Create a buffer for the IPv6 address
	char ipv6Address[INET6_ADDRSTRLEN] = "::ffff:";

	// Check if the input IPv4 address is valid
	struct in_addr ipv4AddrStruct;
	if (inet_pton(AF_INET, ipv4Address.c_str(), &ipv4AddrStruct) != 1)
	{
		std::cerr << "Invalid IPv4 address." << std::endl;
		return "";
	}

	// Convert the binary IPv4 address to its string representation
	char ipv4AddrStr[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &ipv4AddrStruct, ipv4AddrStr, INET_ADDRSTRLEN) == NULL)
	{
		std::cerr << "Failed to convert IPv4 address to string." << std::endl;
		return "";
	}

	// Append the IPv4 address to the IPv6 prefix
	strcat(ipv6Address, ipv4AddrStr);
	return std::string(ipv6Address);
}

void ServerSocket::prepareServerSocketAddr()
{
	std::cout << "Entering prepareServerSocketAddr" << std::endl;
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	// hints.ai_family = _listen.getIsIpv6() ? AF_INET6 : AF_INET;
	hints.ai_family = AF_INET6;
	std::cout << "hints.ai_family: " << hints.ai_family << std::endl;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::string port = toString(_listen.getPort());
	std::cout << "port: " << port << std::endl;
	std::cout << "ip: " << _listen.getIp() << std::endl;
	std::string ip;
	if (!_listen.getIp().empty() && !_listen.getIsIpv6())
	{
		std::cout << "IPv4 address detected" << std::endl;
		std::string ipv6Address = mapIPv4ToIPv6(_listen.getIp());
		std::cout << "ipv6Address: " << ipv6Address << std::endl;
		ip = ipv6Address;
	}
	else
		ip = _listen.getIp();
	std::cout << "ip: " << ip << std::endl;
	// Map IPv4 to IPv6

	int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
	std::cout << "getaddrinfo status: " << status << std::endl;
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
	std::cout << "Before memcpy, ip: " << inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr) << std::endl;
	memcpy(&_serverSocketAddr, res->ai_addr, res->ai_addrlen);
	std::cout << "After memcpy, port: " << ntohs(((struct sockaddr_in *)&_serverSocketAddr)->sin_port) << std::endl;
	std::cout << "After memcpy, ip: " << inet_ntoa(((struct sockaddr_in *)&_serverSocketAddr)->sin_addr) << std::endl;
	freeaddrinfo(res);
	
}
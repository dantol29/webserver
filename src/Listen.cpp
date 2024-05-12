#include "Listen.hpp"

Listen::Listen()
{
	_ip = "";
	_port = DEFAULT_PORT;
	_isIpv6 = false;
	_hasPort = false;
	_hasIP = false;
	Debug::log("Listen default constructor called", Debug::OCF);
}

Listen::Listen(std::string ip, int port, bool ipv6)
{
	_ip = ip;
	_port = port;
	_isIpv6 = ipv6;
	_hasPort = true;
	_hasIP = true;
	Debug::log("Listen param (ip, port, ipv6) constructor called", Debug::OCF);
}

Listen::Listen(std::string str)
{
	std::string ip = "0.0.0.0";
	int port = DEFAULT_PORT;
	std::string portStr;
	_hasPort = false;
	_hasIP = false;
	bool isIpAndPort = _hasPort && _hasIP;
	struct addrinfo hints;
	struct addrinfo *res;

	Debug::log("Listen param (str) constructor called", Debug::OCF);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	 // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// If IPv6:port or [ip]:port format
	if (str[0] == '[')
		str.erase(0, 1);
	if (str.find(']') != std::string::npos)
		str.replace(str.find(']'), 1, "");

	// (IPv6:port) or (IPv6) or (IPv4) or (port)
	// == 0 means success
	if (getaddrinfo(str.c_str(), NULL, &hints, &res) == 0)
	{
		// Immediately free the memory allocated for the addrinfo struct cause we don't need it
		freeaddrinfo(res);
		portStr = str;
		// (IPv6:port)
		if (str.find_last_of(':') != std::string::npos)
		{
			portStr = str.substr(str.find_last_of(':') + 1);

			isIpAndPort = true;
		}

		port = strToInt(portStr);
		if (port >= 1 && port <= 65535)
		{
			_port = port;
			if (!isIpAndPort)
			{
				_ip = "Any";
				return;
			}
		}
		// is incorrect integer
		else if ((port < 1 || port > 65535) && port != -1)
			throw("Invalid port number");

		ip = str;
		// (IPv6:port) or (IPv6)
		if (isIpAndPort)
			ip = str.substr(0, str.find_last_of(':'));
		_ip = ip;
		_isIpv6 = true;
	}
	// (IPv4:port)
	else
	{
		ip = str.substr(0, str.find_last_of(':'));
		portStr = str.substr(str.find_last_of(':') + 1);
		port = strToInt(portStr);
		if (port < 1 || port > 65535)
			throw("Invalid port");
		_ip = ip;

		if (getaddrinfo(ip.c_str(), NULL, &hints, &res) != 0)
			throw("Invalid ip");
		freeaddrinfo(res);
		_port = port;
	}
	if (_ip.empty())
		_ip = "Any";
	if (_port == 0)
		_port = 0;

	return;
}

Listen::Listen(const Listen &obj)
{
	_ip = obj._ip;
	_port = obj._port;
	_isIpv6 = obj._isIpv6;
	Debug::log("Listen copy constructor called", Debug::OCF);
}

Listen &Listen::operator=(const Listen &obj)
{
	_ip = obj._ip;
	_port = obj._port;
	_isIpv6 = obj._isIpv6;
	Debug::log("Listen assignment operator called", Debug::OCF);
	return *this;
}

// Getters
std::string Listen::getIp() const
{
	return _ip;
}

int Listen::getPort() const
{
	return _port;
}

bool Listen::getIsIpv6() const
{
	return _isIpv6;
}

bool Listen::getHasPort() const
{
	return _hasPort;
}

bool Listen::getHasIP() const
{
	return _hasIP;
}

// Setters
void Listen::setIp(std::string ip)
{
	_ip = ip;
}

void Listen::setPort(int port)
{
	_port = port;
}

void Listen::setIsIpv6(bool ipv6)
{
	_isIpv6 = ipv6;
}

void Listen::setHasPort(bool hasPort)
{
	_hasPort = hasPort;
}

void Listen::setHasIP(bool hasIP)
{
	_hasIP = hasIP;
}

// Extract port from string

bool Listen::extractPort(std::string str, int &port, bool &isIpv6)
{
	(void)port;
	size_t lastColon = str.find_last_of(':');
	if (lastColon != std::string::npos)
	{
		// This check assumes IPv6 addresses had brackets removed but may contain more than one colon
		size_t firstColon = str.find(':');
		isIpv6 = (firstColon != lastColon); // Likely an IPv6 if more than one colon remains

		// Extract potential port substring
		std::string portStr = str.substr(lastColon + 1);
		// Find the next space to limit the port string if there's additional text
		size_t spacePos = portStr.find(' ');
		if (spacePos != std::string::npos)
		{
			portStr = portStr.substr(0, spacePos);
		}
	}
	return false; // Return false if no valid port found
}

inline std::ostream &operator<<(std::ostream &os, const Listen &l)
{
	os << "IP: " << l.getIp() << ", Port: " << l.getPort() << ", IPv6: " << (l.getIsIpv6() ? "Yes" : "No");
	return os;
}
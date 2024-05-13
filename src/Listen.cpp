#include "Listen.hpp"

Listen::Listen()
{
	_ip = "";
	_port = DEFAULT_PORT;
	_isIpv6 = false;
	_hasIpOrPort = false;
	_hasPort = false;
	_hasIP = false;
	_hasOptions = false;
	_options = std::vector<std::string>();
	Debug::log("Listen default constructor called", Debug::OCF);
}

Listen::Listen(std::string str)
{
	_ip = DEFAULT_IP;
	_port = DEFAULT_PORT;
	std::string portStr;
	_hasPort = false;
	_hasIP = false;

	Debug::log("Listen param (str) constructor called", Debug::OCF);

	if (listenStrIsEmtpy(str))
		return;
	// normalizeIPv6(str);
	splitInputFromOptions(str);
	if (!_hasIpOrPort)
		return;
	if (inputIsOnlyPort(str) && portIsValid(str))
	{
		_port = strToInt(str);
		_ip = DEFAULT_IP;
		_isIpv6 = false;
		return;
	}
	if (!setIpAndPort(str))
	{
		std::cerr << "Invalid ip or port" << std::endl;
		std::cerr << "Throwing exception" << std::endl;
		throw("Invalid ip or port");
	}
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

bool Listen::listenStrIsEmtpy(std::string &str)
{
	if (str.empty())
	{
		_ip = "0.0.0.0";	  // Default IP address when no input is given
		_port = DEFAULT_PORT; // Default port when no input is given
		_isIpv6 = false;	  // Default to IPv4 when no input is given
		_hasPort = true;
		_hasIP = true;
		return true;
	}
	return false;
}

void Listen::normalizeIPv6(std::string &ip)
{
	if (ip[0] == '[')
		ip.erase(0, 1);
	if (ip.find(']') != std::string::npos)
		ip.replace(ip.find(']'), 1, "");
}

void Listen::splitInputFromOptions(std::string &str)
{
	_options.clear();
	_hasOptions = false;
	_hasIpOrPort = false;

	if (std::isalpha(str[0]))
	{
		_ip = DEFAULT_IP;
		_port = DEFAULT_PORT;
		std::istringstream iss(str);
		std::string option;
		while (iss >> option)
			_options.push_back(option);
		_hasOptions = true;
		_hasIpOrPort = false;
	}
	else
	{
		_hasIpOrPort = true;
		std::size_t firstSpace = str.find(' ');
		if (firstSpace == std::string::npos)
			return;
		else
		{
			_hasOptions = true;
			str = str.substr(0, firstSpace);
			std::istringstream iss(str.substr(firstSpace + 1));
			std::string option;
			while (iss >> option)
				_options.push_back(option);
		}
	}
}

bool strIsAllDigits(const std::string &input)
{
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (!isdigit(input[i])) // Check if the character is not a digit
			return false;		// Return false if any character is not a digit
	}
	return true; // Return true if all characters are digits
}

bool Listen::inputIsOnlyPort(std::string &str)
{
	if (strIsAllDigits(str))
		return true;
	return false;
}

bool Listen::setIpAndPort(std::string &str)
{
	struct addrinfo hints, *res;
	int result;
	std::string portStr;
	std::string defaultPort = toString(DEFAULT_PORT);
	// std::cout << "Before find" << std::endl;
	// std::cout << "str: " << str << std::endl;

	size_t colon = str.rfind(':');

	if (colon > 0 && str[colon - 1] == ':')
	{
		_isIpv6 = true;
		_hasPort = false;
		portStr = defaultPort;
	}
	else
	{
		_isIpv6 = false;
		_hasPort = true;
		if (colon != std::string::npos)
			portStr = str.substr(colon + 1);
	}
	if (portStr.empty())
		portStr = defaultPort;
	// std::cout << "PortStr: " << portStr << std::endl;
	if (!_isIpv6 && _hasPort)
		str = str.substr(0, colon);
	portIsValid(portStr);
	normalizeIPv6(str);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	 // IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// std::cout << "Before getaddrinfo" << std::endl;
	// std::cout << "str: " << str << std::endl;
	// std::cout << "str.c_str(): " << str.c_str() << std::endl;
	result = getaddrinfo(str.c_str(), portStr.c_str(), &hints, &res);
	// std::cout << "After getaddrinfo" << std::endl;
	// std::cout << "Result: " << result << std::endl;

	if (result == 0)
	{
		for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
			if (p->ai_family == AF_INET)
			{
				// std::cout << "IPv4" << std::endl;
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				_ip = inet_ntoa(ipv4->sin_addr);
				// std::cout << "IP: " << _ip << std::endl;
				_port = ntohs(ipv4->sin_port);
				// std::cout << "Port: " << _port << std::endl;
				_isIpv6 = false;
				_hasIP = true;
				if (_port == 0)
					_port = DEFAULT_PORT;
				else
					_hasPort = true;
				break;
			}
			else if (p->ai_family == AF_INET6)
			{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				char ipstr[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET6, &ipv6->sin6_addr, ipstr, sizeof(ipstr));
				_ip = ipstr;
				_port = ntohs(ipv6->sin6_port);
				_isIpv6 = true;
				if (_port == 0)
					_port = DEFAULT_PORT;
				else
					_hasIP = true;
				_hasPort = true;
				break;
			}
		freeaddrinfo(res);
		return true;
	}
	else
	{
		std::cerr << "Error: " << gai_strerror(result) << std::endl;
		return false;
	}
}

bool Listen::portIsValid(std::string &str)
{
	int port = strToInt(str);
	if (port < 1 || port > 65535)
	{
		// std::cerr << "Invalid port number" << std::endl;
		// std::cerr << "Throwing exception" << std::endl;
		throw std::runtime_error("Invalid port number");
		return false;
	}
	return true;
}

inline std::ostream &operator<<(std::ostream &os, const Listen &l)
{
	os << "IP: " << l.getIp() << ", Port: " << l.getPort() << ", IPv6: " << (l.getIsIpv6() ? "Yes" : "No");
	return os;
}
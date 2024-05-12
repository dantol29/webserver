#ifndef LISTEN_HPP
#define LISTEN_HPP

#define DEFAULT_PORT 80

#include <webserv.hpp>
#include <Debug.hpp>

class Listen
{
  private:
	std::string _ip;
	int _port;
	bool _isIpv6;
	bool _hasPort;
	bool _hasIP;

  public:
	Listen();
	Listen(std::string ip, int port, bool ipv6);
	Listen(std::string str);

	Listen(const Listen &obj);
	Listen &operator=(const Listen &obj);

	// Getters
	std::string getIp() const;
	int getPort() const;
	bool getIsIpv6() const;
	bool getHasPort() const;
	bool getHasIP() const;

	// Setters
	void setIp(std::string ip);
	void setPort(int port);
	void setIsIpv6(bool ipv6);
	void setHasPort(bool hasPort);
	void setHasIP(bool hasIP);

	bool extractPort(std::string str, int &port, bool &isIpv6);
};

// Overload the << operator outside of the Listen struct
// The inline keyword is used to avoid multiple definitions of the operator
inline std::ostream &operator<<(std::ostream &os, const Listen &l);

#endif
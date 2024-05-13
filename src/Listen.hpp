#ifndef LISTEN_HPP
#define LISTEN_HPP

#define DEFAULT_PORT 80
#define DEFAULT_IP "0.0.0.0"

#include <arpa/inet.h> // for inet_ntoa
#include <webserv.hpp>
#include <Debug.hpp>
#include <netdb.h>

class Listen
{
  private:
	std::string _ip;
	int _port;
	std::vector<std::string> _options;
	bool _hasIpOrPort;
	bool _hasPort;
	bool _hasIP;
	bool _isIpv6;
	bool _hasOptions;

  public:
	Listen();
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

	bool listenStrIsEmtpy(std::string &str);
	void splitInputFromOptions(std::string &str);
	void normalizeIPv6(std::string &ip);
	// bool extractPort(std::string str, int &port, bool &isIpv6);
	bool inputIsOnlyPort(std::string &str);
	bool portIsValid(std::string &str);
	bool setIpAndPort(std::string &str);
};

// Overload the << operator outside of the Listen struct
// The inline keyword is used to avoid multiple definitions of the operator
inline std::ostream &operator<<(std::ostream &os, const Listen &l);

#endif
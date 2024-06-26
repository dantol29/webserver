#include "server_utils.hpp"
#include <sstream>	// std::istringstream
#include <iostream> // std::cerr
#include "Connection.hpp"

void perrorAndExit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

char customToLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	return c;
}

void printStrWithNonPrintables(const std::string httpRequest, size_t startPos = 0)
{
	std::cout << "HTTP Request (normal std::cout):" << httpRequest << std::endl;
	std::cout << "HTTP Request with not printables:" << std::endl;
	for (size_t i = startPos; i < httpRequest.length(); ++i)
	{
		unsigned char c = httpRequest[i];
		{
			switch (c)
			{
			case '\n':
				std::cout << "\\n";
				break;
			case '\r':
				std::cout << "\\r";
				break;
			case '\t':
				std::cout << "\\t";
				break;
			default:
				if (c >= 32 && c <= 126)
					std::cout << c;
				else
					std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << std::dec;
			}
		}
	}
	std::cout << std::endl;
}

void printVariablesHeadersBody(const HTTPRequest &obj)
{
	std::multimap<std::string, std::string> a = obj.getHeaders();
	std::multimap<std::string, std::string> b = obj.getQueryString();
	std::string c = obj.getBody();

	std::multimap<std::string, std::string>::iterator it;
	std::cout << "Variables: =>" << std::endl;
	for (it = b.begin(); it != b.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	std::cout << "Headers: =>" << std::endl;
	for (it = a.begin(); it != a.end(); it++)
	{
		std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
	}
	// std::cout << "Body: =>" << std::endl;
	std::cout << c << std::endl;
}

void printFrame(std::string frame, bool value)
{
	if (!value)
		return;
	std::cout << ORANGE << BLINKING;
	std::cout << std::setw(10) << ' ' << "***************************" << std::endl;
	std::cout << std::setw(10) << ' ' << "*   " << frame << "   *" << std::endl;
	std::cout << std::setw(10) << ' ' << "***************************" << std::endl;
	std::cout << RESET;
}

void printConnections(const std::string &location,
					  const std::vector<pollfd> &_FDs,
					  const std::vector<Connection> &_connections,
					  bool value)
{
	if (!value)
		return;
	std ::cout << YELLOW << "\n\n++++++++++++++ PRINTING _FDs and _connections VECTORS" << location
			   << "START +++++++++++++++" << RESET << std::endl;
	printFDsVector(_FDs);
	print_connectionsVector(_connections);
	std::cout << YELLOW << "++++++++++++++ PRINTING _FDs and _connections VECTORS" << location << "END +++++++++++++++"
			  << RESET << std::endl;
}

void printFDsVector(const std::vector<pollfd> &fds)
{
	std::cout << CYAN << "++++++ PRINTING _FDs VECTOR START ++++++" << RESET << std::endl;
	std::cout << "SIZE: " << fds.size() << std::endl;
	for (size_t i = 0; i < fds.size(); ++i)
	{
		std::cout << " - #" << i << ": " << "fd: " << fds[i].fd << ", events: " << fds[i].events
				  << ", revents: " << fds[i].revents << std::endl;
	}
	std::cout << CYAN << "++++++ PRINTING _FDs VECTOR END ++++++" << RESET << std::endl;
}

void print_connectionsVector(const std::vector<Connection> &connections)
{
	std::cout << CYAN << "XXX PRINTING _connections VECTOR XXX START XXX" << RESET << std::endl;
	std::cout << "SIZE: " << connections.size() << std::endl;
	std::cout << "CONNECTIONS: ... " << std::endl;
	for (size_t i = 0; i < connections.size(); ++i)
	{
		connections[i].printConnection();
	}
	std::cout << CYAN << "XXX PRINTING _connections VECTOR XXX STOP XXX" << RESET << std::endl;
}
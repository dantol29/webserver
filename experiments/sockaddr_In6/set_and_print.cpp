#include <iostream>
#include <sstream>

#include <cstdio>		// For printf
#include <cstring>		// For memset
#include <netinet/in.h> // For sockaddr_in6
#include <arpa/inet.h>	// For inet_ntop

bool inetPtonIpV6(const std::string &ip, struct sockaddr_in6 &addr)
{
	std::memset(addr, 0, sizeof(*addr));

	if (ip.empty())
	{
		std::cerr << "Error: Empty string" << std::endl;
		return false;
	}

	int colonCount = 0;
	std::size_t start = 0, end = 0;
	std::vector<std::string> parts;

	// Check if there is a "::" in the IP address
	std::size_t doubleColonPos = ip.find("::");
	bool hasDoubleColon = doubleColonPos != std::string::npos;

	// Split the address into parts between colons
	while ((end = ip.find(':', start)) != std::string::npos)
	{
		// Avoid empty parts
		if (start != end)
			parts.push_back(ip.substr(start, end - start));
		else if ((hasDoubleColon) && (end = doubleColonPos))
		{
			parts.push_back("");
		}
		start = end + 1;
		colonCount++;
	}
	// Handle the last part with no colon
	if (start != end)
		parts.push_back(ip.substr(start));
	if (hasDoubleColon && doubleColonPos == ip.size() - 2)
		parts.push_back("");
	if (parts.size() > 8 || (parts.size() == 8 && colonCount > 7))
	{
		std::cerr << "Error: Too many parts" << std::endl;
		return false;
	}

	int skipped_index = -1;
	std::vector<unsigned short> binary_parts(8, 0);

	for (std::size_t i = 0, j = 0; i < parts.size(); ++i)
	{
		if (parts[i].empty())
		{
			if (skipped_index != -1)
				return false; // "::" should only appear once
			skipped_index = j;
			continue;
		}
		std::istringstream iss(parts[i]);
		if (!(iss >> std::hex >> binary_parts[j++]))
			return false;
	}

	if (skipped_index != -1)
	{
		int zeros_to_insert = 8 - (int)parts.size() + 1;
		binary_parts.insert(binary_parts.begin() + skipped_index, zeros_to_insert, 0);
	}

	for (int i = 0; i < 8; ++i)
	{
		addr.sin6_addr.s6_addr[2 * i] = binary_parts[i] >> 8;
		addr.sin6_addr.s6_addr[2 * i + 1] = binary_parts[i] & 0xFF;
	}

	return true;
}

void printSockAddrIn6(const sockaddr_in6 &addr)
{
	char ipv6str[INET6_ADDRSTRLEN]; // Buffer to hold the IPv6 string

	// Convert IPv6 binary address to string
	if (inet_ntop(AF_INET6, &addr.sin6_addr, ipv6str, sizeof(ipv6str)) == NULL)
	{
		std::cerr << "Error converting IPv6 address" << std::endl;
		return;
	}

	// Print the structure components
	std::cout << "IPv6 Address Family: " << addr.sin6_family << std::endl
			  << "IPv6 Port: " << ntohs(addr.sin6_port) << std::endl
			  << "IPv6 Address: " << ipv6str << std::endl
			  << "IPv6 Flow Info: " << addr.sin6_flowinfo << std::endl
			  << "IPv6 Scope ID: " << addr.sin6_scope_id << std::endl;
}

int main()
{
	sockaddr_in6 addr6;

	// Zero out the structure
	memset(&addr6, 0, sizeof(addr6));

	// Setting up a sample sockaddr_in6 structure
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(8080); // Sample port 8080
	addr6.sin6_flowinfo = 0;	   // Typically zero
	addr6.sin6_scope_id = 0;	   // Typically zero

	// Sample IPv6 address, this should be adjusted to your context
	inet_pton(AF_INET6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", &addr6.sin6_addr);

	// Call the print function
	printSockAddrIn6(addr6);

	return 0;
}

#include <iostream>
#include <cassert>
#include "ServerBlock.hpp"

void runTest(const std::string &testname, bool testResult)
{
	if (testResult)
	{
		std::cout << "✅ " << testname << std::endl;
	}
	else
	{
		std::cerr << "❌ " << testname << std::endl;
	}
}

int main()
{
	bool testResult;

	// Test 1: Correct IPv4 and port
	try
	{
		Listen listen1("192.168.1.1:80");
		testResult = (listen1._ip == "192.168.1.1" && listen1._port == 80 && !listen1._isIpv6);
		runTest("Test 1: Correct IPv4 and port", testResult);
	}
	catch (...)
	{
		runTest("Test 1: Correct IPv4 and port", false);
	}
	// Test 2: Correct IPv6 and port
	try
	{
		Listen listen2("[2001:db8::1]:8080");
		testResult = (listen2._ip == "2001:db8::1" && listen2._port == 8080 && listen2._isIpv6);
		runTest("Test 2: Correct IPv6 and port", testResult);
	}
	catch (...)
	{
		runTest("Test 2: Correct IPv6 and port", false);
		// Test 3: IPv6 without port
	}
	try
	{
		Listen listen3("2001:db8::1");
		testResult = (listen3._ip == "2001:db8::1" && listen3._port == 0 && listen3._isIpv6);
		runTest("Test 3: IPv6 without port", testResult);
	}
	catch (...)
	{
		runTest("Test 3: IPv6 without port", false);
	}
	// Test 4: IPv4 without port
	try
	{
		Listen listen4("192.168.1.1");
		testResult = (listen4._ip == "192.168.1.1" && listen4._port == 0 && !listen4._isIpv6);
		runTest("Test 4: IPv4 without port", testResult);
	}
	catch (...)
	{
		runTest("Test 4: IPv4 without port", false);
	}
	try
	{
		// Test 5: Default values
		Listen listen5("");
		testResult = (listen5._ip == "Any" && listen5._port == 0 && !listen5._isIpv6);
		runTest("Test 5: Default values", testResult);
	}
	catch (...)
	{
		runTest("Test 5: Default values", false);
	}
	try
	{
		// Test 6: Invalid port number
		Listen listen6("192.168.1.1:70000");
		// Should throw an exception or handle it internally
	}
	catch (...)
	{
		runTest("Test 6: Invalid port number", true);
	}
	std::cout << "Done testing Listen\n" << std::endl;
	return 0;
}
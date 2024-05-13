#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "ServerBlock.hpp"

enum ExpectedOutcome
{
	Normal,
	Fail,
	Exception
};

struct TestCase
{
	std::string input;
	std::string expectedIP;
	int expectedPort;
	bool expectedIPv6;
	std::string testName;
	ExpectedOutcome outcome;

	TestCase(const std::string &inp,
			 const std::string &ip,
			 int port,
			 bool ipv6,
			 const std::string &name,
			 ExpectedOutcome out = Normal)
		: input(inp), expectedIP(ip), expectedPort(port), expectedIPv6(ipv6), testName(name), outcome(out)
	{
	}
};

void runTest(const TestCase &test)
{
	try
	{
		// std::cout << "--------------------------------\n";
		// std::cout << test.testName << std::endl;
		Listen listen(test.input);
		bool result = (listen.getIp() == test.expectedIP && listen.getPort() == test.expectedPort &&
					   listen.getIsIpv6() == test.expectedIPv6);
		if (result && test.outcome == Normal)
		{
			std::cout << "✅ " << test.testName << std::endl;
		}
		else if (!result && test.outcome == Fail)
		{
			std::cout << "✅ Expected to fail and failed: " << test.testName << std::endl;
		}
		else
		{
			std::cerr << "❌ " << test.testName << " failed: Expected IP " << test.expectedIP << ", Port "
					  << test.expectedPort << ", IPv6 " << (test.expectedIPv6 ? "Yes" : "No") << " - Got IP "
					  << listen.getIp() << ", Port " << listen.getPort() << ", IPv6 "
					  << (listen.getIsIpv6() ? "Yes" : "No") << std::endl;
		}
	}
	catch (const std::runtime_error &e)
	{
		if (test.outcome == Exception || test.outcome == Fail)
		{
			std::cout << "✅ Expected exception and caught: " << test.testName << " - " << e.what() << std::endl;
		}
		else
		{
			std::cerr << "❌ Unexpected exception in " << test.testName << ": " << e.what() << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		if (test.outcome == Exception || test.outcome == Fail)
		{
			std::cout << "✅ Expected exception and caught: " << test.testName << " - " << e.what() << std::endl;
		}
		else
		{
			std::cerr << "❌ Unexpected exception in " << test.testName << ": " << e.what() << std::endl;
		}
	}
	catch (...)
	{
		if (test.outcome == Exception || test.outcome == Fail)
		{
			std::cout << "✅ Expected unknown exception and caught: " << test.testName << std::endl;
		}
		else
		{
			std::cerr << "❌ Unexpected unknown exception in " << test.testName << std::endl;
		}
	}
}

int main()
{
	std::vector<TestCase> tests = {
		TestCase("192.168.1.1:80", "192.168.1.1", 80, false, "Test 1: Correct IPv4 and port"),
		TestCase("[2001:db8::1]:8080", "2001:db8::1", 8080, true, "Test 2: Correct IPv6 and port"),
		TestCase("2001:db8::1", "2001:db8::1", DEFAULT_PORT, true, "Test 3: IPv6 without port"),
		TestCase("192.168.1.1", "192.168.1.1", DEFAULT_PORT, false, "Test 4: IPv4 without port"),
		TestCase("", "0.0.0.0", DEFAULT_PORT, false, "Test 5: Default values"),
		TestCase("localhost", "localhost", DEFAULT_PORT, false, "Test 6: Localhost"),
		TestCase("127.0.0.1:8080", "127.0.0.1", 8080, false, "Test 7: IPv4 with port"),
		TestCase("[::1]:80", "::1", 80, true, "Test 8: IPv6 with port"),
		TestCase("192.168.1.1:70000", "", 0, false, "Test 9: Invalid port number", Fail) // Expected to fail or throw
	};

	for (const auto &test : tests)
	{
		runTest(test);
	}
	std::cout << "Done testing Listen\n" << std::endl;
	return 0;
}

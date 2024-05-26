#include "Server.hpp"
#include "Config.hpp"
#include "webserv.hpp"
#include "EventManager.hpp"
#include "ServerEventListener.hpp"
#include "SSLManager.hpp"
#include "SSLContext.hpp"

int main(int argc, char **argv)
{
	Debug::enable(false);
	Debug::setLevel(Debug::CGI);

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}

	Config config(argv[1]);
	if (!config.getErrorMessage().empty())
		return 1;

	// std::cout << config << std::endl; // should be in the DEBUG?
	EventManager eventManager;
	Server webserv(config, eventManager);

	ServerEventListener serverEventListener(webserv);
	eventManager.subscribe(&serverEventListener);

	// Initialize SSLManager and SSLContext
	// SSLManager *sslManager = SSLManager::getInstance();
	// SSLContext sslContext;

	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
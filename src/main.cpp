#include "Server.hpp"
#include "Config.hpp"
#include "webserv.hpp"
#include "EventManager.hpp"
#include "ServerEventListener.hpp"

int main(int argc, char **argv)
{
	Debug::enable(false);
	Debug::setLevel(Debug::NORMAL);

	if (argc > 2)
	{
		std::cout << "usage: ./webserv [config file]" << std::endl;
		return (1);
	}

	Config config(argv[1]);
	if (!config.getErrorMessage().empty())
		return 1;

	std::cout << config << std::endl; // should be in the DEBUG?
	EventManager eventManager;
	Server webserv(config, eventManager);

	ServerEventListener serverEventListener(webserv);
	std::cout << "Subscribing serverEventListener" << std::endl;
	std::cout << "Pointer to serverEventListener: " << &serverEventListener << std::endl;
	eventManager.subscribe(&serverEventListener);

	std::cout << &webserv.getEventManager() << std::endl;
	std::cout << &eventManager << std::endl;
	std::cout << "SIZE: " << webserv.getEventManager().getObservers().size() << std::endl;
	webserv.startListening();
	webserv.startPollEventLoop();

	return 0;
}
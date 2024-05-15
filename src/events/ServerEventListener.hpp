// ServerEventListener.hpp
#ifndef SERVER_EVENT_LISTENER_HPP
#define SERVER_EVENT_LISTENER_HPP

#include "IEventListener.hpp"
#include "Server.hpp"

class ServerEventListener : public IEventListener
{
	Server &server; // Reference to the server

  public:
	ServerEventListener(Server &srv);
	virtual void handleEvent(int eventID);
};

#endif

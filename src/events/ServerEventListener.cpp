// ServerEventListener.cpp
#include "ServerEventListener.hpp"

ServerEventListener::ServerEventListener(Server &srv) : server(srv)
{
}

void ServerEventListener::handleEvent(const EventData &eventData)
{
	// TODO; Create eventually enum for eventID
	if (eventData.eventType == 1)
	{
		server.addCGI(eventData);
		server.addPipeFDs(eventData.pipe0, eventData.pipe1);
	}
}

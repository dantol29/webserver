// ServerEventListener.cpp
#include "ServerEventListener.hpp"

ServerEventListener::ServerEventListener(Server &srv) : server(srv)
{
}

void ServerEventListener::handleEvent(int eventID)
{
	// TODO; Create eventually enum for eventID
	if (eventID == 1)
		server.addCGI(eventID);
}

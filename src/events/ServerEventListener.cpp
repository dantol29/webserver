// ServerEventListener.cpp
#include "ServerEventListener.hpp"

ServerEventListener::ServerEventListener(Server &srv) : server(srv)
{
}

void ServerEventListener::handleEvent(int eventID)
{
	server.addCGI(eventID);
}

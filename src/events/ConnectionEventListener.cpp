// ServerEventListener.cpp
#include "ConnectionEventListener.hpp"

ConnectionEventListener::ConnectionEventListener(Server &conn) : connection(conn)
{
}

void ConnectionEventListener::handleEvent(const EventData &eventData)
{
	// TODO; Create eventually enum for eventID
	if (eventData.eventType == 1)
		// server.addCGI(eventData);
		connection
}

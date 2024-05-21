// ConnectionEventListener.hpp
#ifndef CONNECTION_EVENT_LISTENER_HPP
#define CONNECTION_EVENT_LISTENER_HPP

#include "IEventListener.hpp"
#include "Connection.hpp"

class ConnectionEventListener : public IEventListener
{
	Connection &connection; // Reference to the server

  public:
	ConnectionEventListener(Connection &conn);
	virtual void handleEvent(const EventData &eventData);
};

#endif

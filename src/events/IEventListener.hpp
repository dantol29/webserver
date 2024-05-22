#ifndef IEVENTLISTENER_HPP
#define IEVENTLISTENER_HPP

#include <iostream>

struct EventData
{
	int eventType;
	int pid;
};

// Overload << operator for easy printing
inline std::ostream &operator<<(std::ostream &os, const EventData &eventData)
{
	os << "Event type: " << eventData.eventType << ", PID: " << eventData.pid;
	return os;
}

class IEventListener
{
  public:
	virtual ~IEventListener()
	{
	}
	virtual void handleEvent(const EventData &eventData) = 0;
};

#endif
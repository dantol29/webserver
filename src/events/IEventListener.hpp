#ifndef IEVENTLISTENER_HPP
#define IEVENTLISTENER_HPP

struct EventData
{
	int eventType;
	int pid;
};

class IEventListener
{
  public:
	virtual ~IEventListener()
	{
	}
	virtual void handleEvent(const EventData &eventData) = 0;
};

#endif
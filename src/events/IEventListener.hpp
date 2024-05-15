#ifndef IEVENTLISTENER_HPP
#define IEVENTLISTENER_HPP

class IEventListener
{
  public:
	virtual ~IEventListener()
	{
	}
	virtual void handleEvent(int eventID) = 0;
};

#endif 
// EventManager.hpp
#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <vector>
#include "IEventListener.hpp"

// EventManager class declaration
class EventManager
{
  private:
	std::vector<IEventListener *> _observers; // List of observers
  public:
	EventManager();
	~EventManager();

	void subscribe(IEventListener *observer);
	void unsubscribe(IEventListener *observer);
	// void emit(int eventID);
	void emit(const EventData &eventData);
};

#endif // EVENT_MANAGER_HPP

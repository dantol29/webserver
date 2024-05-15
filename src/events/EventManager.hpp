// EventManager.hpp
#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <vector>
#include "IEventListener.hpp"

// EventManager class declaration
class EventManager
{
	std::vector<IEventListener *> observers; // List of observers
  public:
	EventManager();
	~EventManager();

	void subscribe(IEventListener *observer);
	void unsubscribe(IEventListener *observer);
	void emit(int eventID);
};

#endif // EVENT_MANAGER_HPP

// EventManager.cpp
#include "EventManager.hpp"
#include <algorithm> // For std::remove

// Constructor
EventManager::EventManager()
{
	// Constructor code here (if needed)
}

// Destructor
EventManager::~EventManager()
{
	// Cleanup code here (if needed)
}

// Subscribe an observer to this manager
void EventManager::subscribe(IEventListener *observer)
{
	observers.push_back(observer);
}

// Unsubscribe an observer from this manager
void EventManager::unsubscribe(IEventListener *observer)
{
	// Remove the observer from the list
	std::vector<IEventListener *>::iterator it = std::remove(observers.begin(), observers.end(), observer);
	observers.erase(it, observers.end());
}

// Emit an event to all registered observers
// void EventManager::emit(int eventID)
void EventManager::emit(const EventData &eventData)
{
	// Notify all observers about the event
	for (std::vector<IEventListener *>::iterator it = observers.begin(); it != observers.end(); ++it)
	{
		(*it)->handleEvent(eventData);
	}
}

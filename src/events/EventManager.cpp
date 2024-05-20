// EventManager.cpp
#include "EventManager.hpp"
#include <algorithm> // For std::remove
#include <iostream>	 // For std::cout

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
	_observers.push_back(observer);
}

// Unsubscribe an observer from this manager
void EventManager::unsubscribe(IEventListener *observer)
{
	// Remove the observer from the list
	std::vector<IEventListener *>::iterator it = std::remove(_observers.begin(), _observers.end(), observer);
	_observers.erase(it, _observers.end());
}

// Emit an event to all registered observers
// void EventManager::emit(int eventID)
void EventManager::emit(const EventData &eventData)
{
	std::cout << "Event emitted: " << eventData << std::endl;
	// Notify all observers about the event
	for (std::vector<IEventListener *>::iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		(*it)->handleEvent(eventData);
	}
}

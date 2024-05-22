// EventManager.cpp
#include "EventManager.hpp"
#include "webserv.hpp"
#include <algorithm> // For std::remove
#include <iostream>	 // For std::cout
#include "ServerEventListener.hpp"
// Constructor
EventManager::EventManager()
{
	Debug::log("EventManager created", Debug::OCF);
}

// Destructor
EventManager::~EventManager()
{
	Debug::log("EventManager destroyed", Debug::OCF);
}

std::vector<IEventListener *> EventManager::getObservers() const
{
	return _observers;
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
	std::cout << YELLOW << "Event emitted: " << eventData << RESET << std::endl;
	// Notify all observers about the event
	std::cout << "Size of observers: " << _observers.size() << std::endl;
	for (std::vector<IEventListener *>::iterator it = _observers.begin(); it != _observers.end(); ++it)
	{
		ServerEventListener *serverEventListener = dynamic_cast<ServerEventListener *>(*it);
		(*it)->handleEvent(eventData);
		std::cout << RED << serverEventListener->getServer().getCGICounter() << RESET << std::endl;
		_pipeFDs = serverEventListener->getServer().getPipeFDs();
	}
}
// clang-format off
std::vector<std::pair<int, int> > EventManager::getPipeFDs() const
{
	return _pipeFDs;
}
//clang-format on
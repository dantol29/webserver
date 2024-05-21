// EventManager.hpp
#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <vector>
#include "webserv.hpp"
#include "IEventListener.hpp"

// EventManager class declaration
class EventManager
{
  private:
	std::vector<IEventListener *> _observers; // List of observers
											  // clang-format off
	std::vector<std::pair<int, int> > _pipeFDs;
 
  public:
	EventManager();
	~EventManager();

	std::vector<IEventListener *> getObservers() const;

	void subscribe(IEventListener *observer);
	void unsubscribe(IEventListener *observer);
	// void emit(int eventID);
	void emit(const EventData &eventData);
	std::vector<std::pair<int, int> > getPipeFDs() const;
											  // clang-format on
};

#endif // EVENT_MANAGER_HPP

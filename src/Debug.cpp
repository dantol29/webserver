#include "Debug.hpp"

// In C++98 the initialization of static members must be done outside the class definition.
bool Debug::debugEnabled = true;
Debug::Level Debug::debugLevel = Debug::NORMAL;

Debug::Debug()
{
}
Debug::Debug(const Debug &)
{
}
Debug &Debug::operator=(const Debug &)
{
	return *this;
}
Debug::~Debug()
{
}

void Debug::enable(bool on)
{
	debugEnabled = on;
}

void Debug::setLevel(Debug::Level level)
{
	debugLevel = level;
}

void Debug::addLevel(Debug::Level level)
{
	debugLevel = static_cast<Debug::Level>(debugLevel | level);
}

void Debug::removeLevel(Debug::Level level)
{
	debugLevel = static_cast<Debug::Level>(debugLevel & ~level);
}

void Debug::log(const std::string &message, Debug::Level paramLevel)
{
	if (!debugEnabled)
	{
		return;
	}
	if (debugLevel & paramLevel)
	{
		std::cout << message << std::endl;
	}
}

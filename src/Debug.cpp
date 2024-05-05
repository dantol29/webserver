#include "Debug.hpp"

bool Debug::debugEnabled = false;
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

void Debug::log(const std::string &message, Debug::Level paramLevel)
{
	if (!debugEnabled)
	{
		return;
	}
	if (debugEnabled && (debugLevel >= paramLevel))
	{
		std::cout << message << std::endl;
	}
}

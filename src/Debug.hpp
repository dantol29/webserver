#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <string>

class Debug
{
  public:
	enum Level
	{
		NORMAL,
		OCF
	};

  private:
	static bool debugEnabled;
	static Level debugLevel;

  public:
	Debug();
	Debug(const Debug &);
	Debug &operator=(const Debug &);
	virtual ~Debug();

	static void enable(bool on);
	static void setLevel(Level level);
	static void log(const std::string &message, Debug::Level paramLevel);
};

#endif

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <string>

class Debug
{
  public:
	enum Level
	{
		NORMAL = 0,
		OCF = 1 << 0,	 // 0000 0001
		SERVER = 1 << 1, // 0000 0010
		PARSER = 1 << 3, // 0000 0100
		CGI = 1 << 4,	 // 0000 1000
		EVENTS = 1 << 5, // 0001 0000
		EVAL = 1 << 6,	 // 0010 0000
		ALL = 1 << 7	 // 0100 0000
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
	static void addLevel(Level level);
	static void removeLevel(Level leve);
	static void log(const std::string &message, Debug::Level paramLevel);
};

#endif

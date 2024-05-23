#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>
#include <iomanip>
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
		ALL = 0xFF		 ///  1111 1111
	};

  private:
	Debug();
	Debug(const Debug &);
	Debug &operator=(const Debug &);
	~Debug();
	static bool debugEnabled;
	static Level debugLevel;

  public:
	static void enable(bool value);
	static void disable(bool off);
	static void setLevel(Level level);
	static void addLevel(Level level);
	static void removeLevel(Level leve);
	// static void log(const std::string &message, Debug::Level paramLevel);
	static void log(const std::string &message,
					Debug::Level paramLevel,
					const std::string &color = "\033[0m",
					bool bliking = false,
					bool frame = false);
};

#endif

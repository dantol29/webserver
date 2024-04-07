#include "webserv.hpp"
#include <string>

#include <string>

bool startsWith(const std::string& fullString, const std::string& starting) {
    if (fullString.length() >= starting.length()) {
        return fullString.substr(0, starting.length()) == starting;
    } else {
        return false;
    }
}

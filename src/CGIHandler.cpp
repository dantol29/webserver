#include "CGIHandler.hpp"
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

CGIHandler::CGIHandler()
{
}

CGIHandler::~CGIHandler()
{
}

HTTPResponse CGIHandler::handleRequest(const HTTPRequest &request)
{
	// Determine CGI script path and set up environment
}
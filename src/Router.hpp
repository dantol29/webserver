#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StaticContentHandler.hpp"
#include "CGIHandler.hpp"
#include "sys/stat.h"

struct resourcePath
{
	std::vector<std::string> directories;
	std::string resource;
};

class Router
{
  public:
	Router();
	~Router();
	HTTPResponse routeRequest(const HTTPRequest &request);

	void splitTarget(const std::string &target);
	bool isDynamicRequest(const HTTPRequest &request);
	bool pathIsValid(HTTPRequest &request, std::string webRoot);

  private:
	Router(const Router &other);
	Router &operator=(const Router &other);
	StaticContentHandler _staticContentHandler;
	CGIHandler _cgiHandler;

	std::string getFileExtension(const std::string &fileName);
	bool isCGI(const HTTPRequest &request);
	resourcePath _path;
};

#endif
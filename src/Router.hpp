#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StaticContentHandler.hpp"
#include "CGIHandlder.hpp"

struct resourcePath
{
	std::vector<std::string> directories;
	std::string resource;
}

class Router
{
  public:
	Router();
	~Router();
	HTTPResponse routeRequest(const HTTPRequest &request);

	void splitTarget(const std::string &target);

  private:
	Router(const Router &other);
	Router &operator=(const Router &other);
	StaticContentHandler _staticContentHandler;
	CGIHandler _cgiHandler;

	bool isDynamicRequest(const HTTPRequest &request);
	bool pathExists(HTTPResponse &response, const std::string &path);
	std::string getFileExtension(const std::string &fileName);
	resourcePath _path;
};

#endif
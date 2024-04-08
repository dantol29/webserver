#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StaticContentHandler.hpp"
#include "CGIHandlder.hpp"

class Router
{
  public:
	Router();
	~Router();
	HTTPResponse routeRequest(const HTTPRequest &request);

  private:
	Router(const Router &other);
	Router &operator=(const Router &other);
	StaticContentHandler _staticContentHandler;
	CGIHandler _cgiHandler;

	bool isDynamicRequest(const HTTPRequest &request);
	std::string getFileExtension(const std::string &fileName);
};

#endif // ROUTER_HPP
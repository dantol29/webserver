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
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	std::vector<struct pollfd> *getFDsRef();

  private:
	Router(const Router &other);
	Router &operator=(const Router &other);
	StaticContentHandler _staticContentHandler;
	CGIHandler _cgiHandler;
	std::vector<struct pollfd> *_FDsRef; // Pointer to store reference to server's _FDs vector
	std::vector<pollfd> *_FDsRef;

	std::string getFileExtension(const std::string &fileName);
	bool isCGI(const HTTPRequest &request);
	resourcePath _path;
};

#endif
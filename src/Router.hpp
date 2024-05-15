#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "EventManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "StaticContentHandler.hpp"
#include "CGIHandler.hpp"
#include "UploadHandler.hpp"
#include "ServerBlock.hpp"
#include "Debug.hpp"
#include "sys/stat.h"
#include <dirent.h> // POSIX lib for DIR

struct resourcePath
{
	std::vector<std::string> directories;
	std::string resource;
};

enum PathValidation
{
	PathValid,
	PathInvalid,
	IsDirectoryListing
};

class Router
{
  public:
	Router(Directives &directive, EventManager &eventManager);
	~Router();
	void routeRequest(HTTPRequest &request, HTTPResponse &response);

	void splitTarget(const std::string &target);
	bool isDynamicRequest(const HTTPRequest &request);
	enum PathValidation pathIsValid(HTTPResponse &response, HTTPRequest &request);
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	void setPollFd(struct pollfd *pollFd);
	void handleServerBlockError(HTTPRequest &request, HTTPResponse &response, int errorCode);

  private:
	Directives _directive;
	Router(const Router &other);
	Router &operator=(const Router &other);
	StaticContentHandler _staticContentHandler;
	CGIHandler _cgiHandler;
	EventManager &_eventManager;
	resourcePath _path;
	std::vector<pollfd> *_FDsRef;
	struct pollfd *_pollFd;
	std::string getFileExtension(const std::string &fileName);
	void generateDirectoryListing(HTTPResponse &Response,
								  const std::string &directoryPath,
								  const std::string &requestedPath);
	bool isCGI(const HTTPRequest &request);
	// We want a reference, cause there is only one instance of the EventManager, which is created in the main function
	void adaptPathForFirefox(HTTPRequest &request);
};

#endif
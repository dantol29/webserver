#ifndef ROUTER_HPP
#define ROUTER_HPP

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
	Router();
	Router(ServerBlock serverBlock);
	~Router();
	Router &operator=(const Router &other);
	void routeRequest(HTTPRequest &request, HTTPResponse &response);

	void splitTarget(const std::string &target);
	bool isDynamicRequest(const HTTPRequest &request);
	enum PathValidation pathIsValid(HTTPResponse &response, HTTPRequest &request);
	void setFDsRef(std::vector<struct pollfd> *FDsRef);
	void setPollFd(struct pollfd *pollFd);
	void setWebRoot(std::string &webRoot);
	void handleServerBlockError(HTTPRequest &request, HTTPResponse &response, int errorCode);

  private:
	ServerBlock _serverBlock;
	Router(const Router &other);
	StaticContentHandler _staticContentHandler;
	resourcePath _path;
	std::vector<pollfd> *_FDsRef;
	struct pollfd *_pollFd;
	std::string getFileExtension(const std::string &fileName);
	void
	generateDirectoryListing(HTTPResponse Response, const std::string &directoryPath, const std::string &requestedPath);
	bool isCGI(const HTTPRequest &request);
	CGIHandler _cgiHandler;
	void adaptRequestForFirefox(HTTPRequest &request);
};

#endif
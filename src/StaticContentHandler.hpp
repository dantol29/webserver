#ifndef STATICCONTENTHANDLER_HPP
#define STATICCONTENTHANDLER_HPP

#include "ARequestHandler.hpp"

class StaticContentHandler : public ARequestHandler
{
  public:
	StaticContentHandler(const std::string &webRoot);
	~StaticContentHandler();
	HTTPResponse handleRequest(const HTTPRequest &request);

  private:
	StaticContentHandler();
	StaticContentHandler(const StaticContentHandler &other);
	StaticContentHandler &operator=(const StaticContentHandler &other);
	std::string _webRoot;
};

#endif // STATICCONTENTHANDLER_HPP
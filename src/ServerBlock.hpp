#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "webserv.hpp"
#include "Listen.hpp"

// a list of all accepted variables
// ---------------------------------
// 1. listen, 2. server_name, 3. error_page,
// 4. index, 5. root, 6. client_max_body_size, 7. autoindex, 
// 8. allow_methods, 9. alias, 10. cgi_path, 11. cgi_ext
// ---------------------------------

struct Directives
{
	Directives()
	{
		_listen.clear();
		_serverName.clear();
		_errorPage.clear();
		_index.clear();
		_root = "";
		_clientMaxBodySize = 0;
		_autoindex = false;
		_allowedMethods.clear();
		_alias = "";
		_cgiPath = "";
		_cgiExt.clear();
		_return = "";
		_uploadPath = "";
		_limit_conn = 0;
		_path = "";
	
	}

	void setListenEntry(Listen listenEntry, bool isLocation);

	std::vector<Listen> _listen;
	std::vector<std::string> _serverName;
	std::vector<std::pair<int, std::string> >_errorPage;
	std::vector<std::string> _index;
	std::string _root;
	size_t _clientMaxBodySize;
	bool _autoindex;
	std::vector<std::string> _allowedMethods;
	std::string _alias;
	std::vector<std::string> _cgiExt;
	std::string _cgiPath;
	std::string _return;
	std::string _uploadPath;
	size_t _limit_conn;
	std::string _path; // only for location blocks
};

class ServerBlock
{
  public:
	ServerBlock();
	ServerBlock(const ServerBlock &obj);
	ServerBlock &operator=(const ServerBlock &obj);
	~ServerBlock();

		bool addDirective(std::string key, std::string& value, bool isLocation);

		// GETTERS
		Directives getDirectives() const; // variables outside of locations
		std::vector<Directives> getLocations() const; // location / {} blocks
		std::vector<Listen> getListen() const;
		std::vector<std::string> getServerName() const;
		std::vector<std::pair<int, std::string> > getErrorPage() const;
		std::vector<std::string> getIndex() const;
		std::string getRoot() const;
		size_t getClientMaxBodySize() const;
		bool getAutoIndex() const;
		std::vector<std::string> getAllowedMethods() const;
		std::string getAlias() const;
		std::vector<std::string> getCgiExt() const;
		std::string getCgiPath() const;
		std::string getReturn() const;
		std::string getUploadPath() const;
		size_t getLimitConn() const;

		// SETTERS
		void setListen(Listen str, bool isLocation);
		void setServerName(std::vector<std::string> str, bool isLocation);
		void setErrorPage(std::pair<int, std::string> str, bool isLocation);
		void setIndex(std::vector<std::string> str, bool isLocation);
		void setRoot(std::string& str, bool isLocation);
		void setClientMaxBodySize(std::string& n, bool isLocation);
		void setAutoIndex(std::string& str, bool isLocation);
		void setAllowedMethods(std::vector<std::string> str, bool isLocation);
		void setAlias(std::string& str, bool isLocation);
		void setCgiExt(std::vector<std::string> str, bool isLocation);
		void setCgiPath(std::string str, bool isLocation);
		void setReturn(std::string str, bool isLocation);
		void setUploadPath(std::string str, bool isLocation);
		void setLimitConn(std::string str, bool isLocation);
		void setLocationPath(std::string str);
		
		// clear ServerBlock
		void deleteData();
	private:
		Directives _directives;
		std::vector<Directives> _locations;
		
		// TRANSFORMERS
		void transformServerListen(std::string& str, bool isLocation);
		std::vector<std::string> transformServerName(std::string& str);
		std::pair<int, std::string> transformErrorPage(std::string& str);
		std::vector<std::string> transformIndex(std::string& str);
		std::vector<std::string> transformAllowedMethods(std::string& str);
		std::vector<std::string> transformCgiExt(std::string& str);

		Listen makeListenStruct(std::string& newStr);
		

};

#endif
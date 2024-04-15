## NGINX grammar
```
server {
	[TAB][KEY][SP][VALUE][;] // variables

	location /path {
		[TAB][TAB][KEY][SP][VALUE][;] // location variables
	}
}
```

## Valid variables
- listen (**mandatory**)
- host
- server_name
- error_page
- index
- root
- client_max_body_size
- autoindex
- allow_methods
- alias
- cgi_path
- cgi_ext

## Valid variables values
- listen [port_number]
- host [???]
- server_name [example.com] [www.example.com]
- error_page [500] [502] [503] [504] [/custom_50x.html]
- index [index.html] [index.htm] [index.php;] // will first look for index.html. If that file isn't found, it will look for index.htm, and if that isn't found ...
- root [/var/www/example.com/html]
- client_max_body_size [20000]
- autoindex [on]
- allow_methods [GET] [POST]
- alias [/var/www/documents/]
- cgi_path [/usr/bin/python3]
- cgi_ext [.py] [.pl]

## How to work with the ConfigFile.hpp
- to initialize just pass path to the ConfigFile(path) constructor
- all the variables are stored here std::map<std::string, std::string> _variables;
- you can get them with std::map<std::string, std::string> getVariables() or std::pair<std::string, std::string> getVariables(std::string name)
- all the "location" variables are stored here std::vector<std::map<std::string, std::string>> _locations;
- you can get them with std::vector<std::map<std::string, std::string> > getLocations();
- you can get an error message with std::string getErrorMessage();. If it returns an empty string it means that config file is valid
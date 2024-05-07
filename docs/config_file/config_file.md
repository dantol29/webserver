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
- listen
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
- server_name [example.com] [www.example.com]
- error_page [500] [/custom_500.html]
- index [index.html] [index.htm] [index.php;] // looks for first valid page
- root [/var/www/example.com/html]
- client_max_body_size [20000]
- autoindex [on]
- allow_methods [GET] [POST]
- alias [/var/www/documents/]
- cgi_path [/usr/bin/python3]
- cgi_ext [.py] [.pl]

### SERVER_NAME
- Can be written only once per _server block_
- A list of server names(strings) [example.com] [www.example.com] ...
- is stored in the `std::vector<std::string>`
- _OUR PROTECTION:_
	1. no protection

### ERROR_PAGE
- Can be written multiple times in the config file
- First value is [INT], second is the [PATH] to the html page
- is stored in the `std::vector<std::pair<int, std::string> >`
- _OUR PROTECTION:_
	1. check if [INT] is valid error number
	2. check if [PATH] exists

### INDEX
- Can be written only once per _server block_
- A list of paths to html pages
- Shows the first existing html page
- is stored in the `std::vector<std::string>`
- _OUR PROTECTION:_
	1. check if [PATH] exists

### ROOT
- Can be written only once per _server block_
- A string to the root of the server
- is stored in the `std::string`
- _OUR PROTECTION:_
	1. check if [PATH] exists

### CLIENT_MAX_BODY_SIZE
- Can be written only once per _server block_
- A number that indecates the max size of the HTTP request body
- is stored in the `size_t`
- _OUR PROTECTION:_
	1. check if number is valid

#!/bin/bash

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m' 

URL="http://127.0.0.1:8080/"

is_error=false

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.example.com" http://127.0.0.1:8080/)

if [ "$response" -eq 200 ]; then
	echo -e "$GREEN www.example.com:8080: 200 $RESET"
else
	echo -e "$RED www.example.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.example.com" http://127.0.0.1:8081/)

if [ "$response" -eq 404 ]; then
	echo -e "$GREEN www.example.com:8081: 404 $RESET"
else
	echo -e "$RED www.example.com:8081: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.php_site.com" http://127.0.0.1:8081/)

if [ "$response" -eq 200 ]; then
	echo -e "$GREEN www.php_site.com:8081: 200 $RESET"
else
	echo -e "$RED www.php_site.com:8081: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.php_site.com" http://127.0.0.1:8080/)

if [ "$response" -eq 404 ]; then
	echo -e "$GREEN www.php_site.com:8080: 404 $RESET"
else
	echo -e "$RED www.php_site.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.python_site.com" http://127.0.0.1:8080/)

if [ "$response" -eq 405 ]; then
	echo -e "$GREEN www.python_site.com:8080: 405(Method not allowed) $RESET"
else
	echo -e "$RED www.python_site.com:8080: $response $RESET"
	is_error=true
fi

# Generate a 2KB string of 'a' characters
data=$(head -c 2048 < /dev/zero | tr '\0' 'a')

# Send the POST request and capture the response code
response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.python_site.com" -H "Content-Type: text/plain" -d "$data" http://127.0.0.1:8080/)

if [ "$response" -eq 413 ]; then
	echo -e "$GREEN www.python_site.com:8080: 413(Payload too large) $RESET"
else
	echo -e "$RED www.python_site.com:8080: $response $RESET"
fi

if [ "$is_error" = true ]; then
    exit 1
fi

exit 0
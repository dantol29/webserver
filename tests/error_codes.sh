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

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/error.cgi)

if [ "$response" -eq 500 ]; then
	echo -e "$GREEN www.development_site.com:8080: 500(Internal Server Error) $RESET"
else
	echo -e "$RED www.development_site.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/permission.cgi)

if [ "$response" -eq 500 ]; then
	echo -e "$GREEN www.development_site.com:8080: 500(Internal Server Error) $RESET"
else
	echo -e "$RED www.development_site.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/hello_var.cgi)

if [ "$response" -eq 200 ]; then
	echo -e "$GREEN www.development_site.com:8080: 200 $RESET"
else
	echo -e "$RED www.development_site.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/duration_ts.cgi)

if [ "$response" -eq 200 ]; then
	echo -e "$GREEN www.development_site.com:8080: 200 $GREEN"
else
	echo -e "$RED www.development_site.com:8080: $response $RESET"
	is_error=true
fi

response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/500seconds.cgi)

if [ "$response" -eq 504 ]; then
	echo -e "$GREEN www.development_site.com:8080: 504(Gateway Timeout) $GREEN"
else
	echo -e "$RED www.development_site.com:8080: $response $RESET"
	is_error=true
fi

urls=(
  "http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"
  "http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"
  "http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"
  "http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"
)

# send 4 requests in parallel (limit in server block is 3, so the last request expects a 503 response)
expected_responses=(200 200 200 503)

for i in "${!urls[@]}"; do
  (
    response=$(curl -s -o /dev/null -w "%{http_code}" -H "Host: www.development_site" "${urls[$i]}")
    if [ "$response" -eq "${expected_responses[$i]}" ]; then
      echo -e "$GREEN www.development_site.com:8080: $response $GREEN"
    else
      echo -e "$RED www.development_site.com:8080: $response $GREEN"
      is_error=true
    fi
  ) &
  
  # Add a delay of 1 second before the next iteration, except after the last request
  if [ "$i" -lt $(( ${#urls[@]} - 1 )) ]; then
    sleep 0.3
  fi
done

# Wait for all background processes to finish
wait

if [ "$is_error" = true ]; then
    exit 1
fi

exit 0
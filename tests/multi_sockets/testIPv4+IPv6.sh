#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if the webserv binary exists
if [ ! -f ./../../webserv ]; then
    echo -e "${RED}Error: webserv binary not found!${NC}"
    exit 1
fi

echo "Starting test script..."

# Modify /etc/hosts to add the new localhost aliases
echo "127.0.0.2 localhost2" >> /etc/hosts
echo "127.0.0.3 localhost3" >> /etc/hosts
echo "::2 localhost2" >> /etc/hosts
echo "::3 localhost3" >> /etc/hosts

# Start the server
echo "Starting the server..."
./../../webserv ./../../multiIPv4+IPv6.conf &
SERVER_PID=$!
sleep 2  # Wait for the server to start

# Check if the server started successfully
if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}Failed to start the server.${NC}"
    exit 1
fi

# Function to store response
store_response() {
    local url=$1
    local expected=$2
    local response=$(curl -s -o /dev/null -w "%{http_code}" $url)
    if [ "$response" == "$expected" ]; then
        echo -e "$url - Expected $expected - HTTP status code: $response ${GREEN}✔${NC}" >> results.txt
    else
        echo -e "$url - Expected $expected - HTTP status code: $response ${RED}✘${NC}" >> results.txt
    fi
}

# Run curl commands and store responses
store_response "127.0.0.1:8080" "404"
store_response "0.0.0.0:8081" "404"
store_response "127.0.0.1:8082" "404"
store_response "127.0.0.2:8080" "000"
store_response "127.0.0.3:8080" "000"
store_response "127.0.0.1:8081" "404"
store_response "[::1]:8080" "404"
store_response "[::]:8081" "404"
store_response "[::1]:8082" "404"
store_response "[::2]:8080" "000"
store_response "[::3]:8080" "000"
store_response "[::1]:8081" "404"

# Stop the server
echo "Stopping the server..."
kill $SERVER_PID

# Print all responses
echo "Test results:"
cat results.txt
rm results.txt

echo "Test script completed."

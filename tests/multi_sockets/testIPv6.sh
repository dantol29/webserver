#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Install net-tools if not present
if ! command -v netstat &> /dev/null; then
    echo "Installing net-tools..."
    apt-get update && apt-get install -y net-tools
fi

# Check if the webserv binary exists
if [ ! -f ./../../webserv ]; then
    echo -e "${RED}Error: webserv binary not found!${NC}"
    exit 1
fi

echo "Starting test script..."

# Modify /etc/hosts to add the new localhost aliases
echo "::2 localhost2" >> /etc/hosts
echo "::3 localhost3" >> /etc/hosts

# Start the server
echo "Starting the server..."
./../../webserv ./../../multiIPv6.conf &
SERVER_PID=$!
sleep 2  # Wait for the server to start

# Check if the server started successfully
if ! ps -p $SERVER_PID > /dev/null; then
    echo -e "${RED}Failed to start the server.${NC}"
    exit 1
fi

# Verify server is listening on the expected address and port
echo "Checking server listening ports..."
netstat -tuln | grep 8081 >> netstat_results.txt
netstat -tuln | grep 8080 >> netstat_results.txt
netstat -tuln | grep 8082 >> netstat_results.txt

# Check IPv6 configuration
echo "IPv6 configuration:"
ip -6 addr > ipv6_config.txt
ip -6 route >> ipv6_config.txt


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
store_response "[::1]:8080" "404"
# store_response "[::]:8081" "404"
store_response "[::0]:8081" "404"
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

# Print netstat results
echo "Netstat results:"
cat netstat_results.txt
rm netstat_results.txt

# Print IPv6 configuration
echo "IPv6 configuration:"
cat ipv6_config.txt
rm ipv6_config.txt



echo "Test script completed."

#!/bin/bash

NAME=./webserv

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m' 

echo -e "$BLUE Welcome to the configuration file TESTER!$RESET"

if test -f $NAME; then
	echo ""
else
	echo -e "$RED Webserver no found...\n$RESET"
	exit;
fi

echo -e "$PURPLE Invalid configuration files:$RESET"
for i in {1..21}; do
	echo -e "config$i"
	timeout 1 $NAME "config_files/config$i" >/dev/null 2>&1
	if [ $? -eq 124 ]; then
		echo -e "${RED}KO${RESET}\n"
	else
		echo -e "${GREEN}OK${RESET}\n"
	fi
	sleep 0.2
done


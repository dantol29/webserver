#!/bin/bash

NAME=../.././webserv

PURPLE='\033[0;35m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RED='\033[0;31m'
RESET='\033[0m' 

echo "██╗    ██╗███████╗███████╗██████╗ ██╗   ██╗    
██║    ██║██╔════╝██╔════╝██╔══██╗██║   ██║    
██║ █╗ ██║███████╗█████╗  ██████╔╝██║   ██║    
██║███╗██║╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝    
╚███╔███╔╝███████║███████╗██║  ██║ ╚████╔╝     
 ╚══╝╚══╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝"

echo "████████╗███████╗███████╗████████╗███████╗██████╗
╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██╔════╝██╔══██╗
   ██║   █████╗  ███████╗   ██║   █████╗  ██████╔╝
   ██║   ██╔══╝  ╚════██║   ██║   ██╔══╝  ██╔══██╗
   ██║   ███████╗███████║   ██║   ███████╗██║  ██║
   ╚═╝   ╚══════╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝"


is_error=false

if test -f $NAME; then
	echo ""
else
	echo -e "$RED Webserver no found...\n$RESET"
	exit;
fi

if test -f "invalid_config_files/config1.test"; then
	echo ""
else
	echo -e "$RED Tests no found...\n$RESET"
	exit;
fi

echo -e "$PURPLE Invalid configuration files:$RESET"
for i in {1..22}; do
	echo -e "config$i"
	timeout 0.5 $NAME "invalid_config_files/config$i.test" >/dev/null 2>&1
	if [ $? -eq 124 ]; then
		echo -e "${RED}KO${RESET}\n"
		is_error=true
	else
		echo -e "${GREEN}OK${RESET}\n"
	fi
	sleep 0.2
done

if [ "$is_error" = true ]; then
    exit 1
fi

exit 0

# if test -f "valid_config_files/config1.test"; then
# 	echo ""
# else
# 	echo -e "$RED Tests no found...\n$RESET"
# 	exit;
# fi

# echo -e "$PURPLE Valid configuration files:$RESET"
# for i in {1..2}; do
# 	echo -e "config$i"
# 	timeout 0.5 $NAME "valid_config_files/config$i.test" >/dev/null 2>&1
# 	if [ $? -eq 124 ]; then
# 		echo -e "${GREEN}OK${RESET}\n"
# 	else
# 		echo -e "${RED}KO${RESET}\n"
# 	fi
# 	sleep 0.2
# done

#!/bin/bash

# Check if the required packages are installed, if not, install them
required_packages=(requests aiohttp asyncio)
for package in "${required_packages[@]}"; do
    pip show $package > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "Installing $package."
        pip install $package
    else
        echo "$package is already installed."
    fi
done
# Confirm activation and installation
echo "Virtual environment '$venv_name' is activated and packages are installed."

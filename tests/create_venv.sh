#!/bin/bash

# Define the name of the virtual environment
venv_name="tests_venv"

# Check if the virtual environment already exists
if [ -d "$venv_name" ]; then
    echo "Virtual environment '$venv_name' already exists."
else
    # Create the virtual environment
    echo "Creating virtual environment '$venv_name'."
    python3 -m venv "$venv_name"
fi

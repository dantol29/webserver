# Python Virtual Environment Setup

This README outlines the steps to create, activate, and install necessary packages into a Python virtual environment called `python_tests`.

## Prerequisites

Ensure you have Python and `pip` installed on your system. This setup is intended for Unix-like operating systems such as Linux and macOS.

## Scripts

There are two scripts provided:

1. **create_venv.sh** - This script creates a virtual environment if it doesn't already exist.
2. **install_packages.sh** - This script installs the required Python packages into the activated virtual environment.

## Setup Instructions

Follow these steps to set up your virtual environment:

### Step 1: Create the Virtual Environment

Run the `create_venv.sh` script to create the virtual environment. This only needs to be done once unless you delete the environment.

```bash
./create_venv.sh
```

### Step 2: Activate the Virtual Environment

Manually activate the virtual environment using the source command. This needs to be done each time you start a new shell session and want to use the virtual environment.

```bash
source tests_venv/bin/activate
```

### Step 3: Install Required Packages

Once the environment is activated, run the `install_packages.sh` script to install the necessary Python packages.

```bash
./install_packages.sh
```

## Usage

After activation, you can use the virtual environment to run Python scripts or projects that depend on the installed packages. Remember, you need to reactivate the environment in each new shell session.

`python3 general_tests.py`

## Deactivation

To deactivate the virtual environment and return to your global Python environment, simply run:

```bash
deactivate
```

## Additional Notes

- Ensure each script is executable: `chmod +x script_name.sh`
- Modify the list of packages in `install_packages.sh` as needed for your projects.

```

```

#!/usr/bin/env python3

import cgi
import os
import json
import sys
from urllib.parse import parse_qs

def get_script_directory():
    """Return the directory in which the script is located."""
    return os.path.dirname(os.path.realpath(__file__))

def load_database(filename):
    """Load the JSON database from a file."""
    print(f"DEBUG: Attempting to load database from {filename}<br>")
    if os.path.exists(filename):
        try:
            with open(filename, "r") as file:
                return json.load(file)
        except json.JSONDecodeError:
            print("Error: Database file contains invalid JSON. Unable to load data.")
            return None
        except Exception as e:
            print(f"Error: Failed to read the database file. {e}")
            return None
    else:
        print(f"DEBUG: Database file {filename} does not exist.<br>")
        return {}

def save_database(data, filename):
    """Save the JSON database to a file."""
    try:
        with open(filename, "w") as file:
            json.dump(data, file, indent=4)
        print(f"DEBUG: Database saved to {filename}<br>")
    except Exception as e:
        print(f"Error: Failed to save the database file. {e}")

def initialize_database(filename):
    """Initialize the database file if it does not exist."""
    if not os.path.exists(filename):
        data = {}
        save_database(data, filename)
        return data
    else:
        return load_database(filename)

def main():
    print("Content-type: text/html\n")
    print()

    # Print the environment variables for debugging
    for key, value in os.environ.items():
        print(f"DEBUG: {key}={value}<br>")
    print("<br><br>")

    # Get the directory where the script is located
    script_directory = get_script_directory()
    filename = os.path.join(script_directory, "database.json")
    print(f"DEBUG: Database filename is {filename}<br>")

    # Initialize and load existing data
    data = initialize_database(filename)
    if data is None:
        data = {}
    print(f"DEBUG: Loaded data: {data}<br>")

    # Determine the request method
    method = os.getenv('REQUEST_METHOD', '').upper()

    if method in ['POST', 'DELETE']:
        if method == 'POST':
            # Read the input data from stdin
            content_length = int(os.getenv('CONTENT_LENGTH', 0))
            post_data = sys.stdin.read(content_length)
            print(f"DEBUG: Received POST data: {post_data}<br>")
            form_data = parse_qs(post_data)
        else:  # DELETE method
            form_data = sys.stdin.read()
        
        # Debugging: Print the form keys and values
        for key in form_data.keys():
            print(f"DEBUG: form[{key}]={form_data[key]}<br>")

        print(f"DEBUG: form_data={form_data}<br>")

        name = form_data.get('name', [None])[0]
        salad = form_data.get('salad', [None])[0] if method == 'POST' else None

        print(f"DEBUG: method={method}, name={name}, salad={salad}<br>")

        if method == 'POST' and name and salad:
            # Add or update an entry
            data[name] = salad
            save_database(data, filename)
            print(f"Entry added or updated successfully: {name} likes {salad}.")
        elif method == 'DELETE' and name:
            # Attempt to delete an entry
            if name in data:
                del data[name]
                save_database(data, filename)
                print(f"Entry deleted successfully: {name}.")
            else:
                print(f"Entry not found: {name}.")
        else:
            print("Invalid request. Make sure you provide name and salad parameters for adding, or name for deleting.")
    else:
        print("Unsupported request method. Please use POST or DELETE.")

if __name__ == "__main__":
    main()

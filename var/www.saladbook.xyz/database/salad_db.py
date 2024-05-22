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
    log(f"Attempting to load database from {filename}")
    if os.path.exists(filename):
        try:
            with open(filename, "r") as file:
                return json.load(file)
        except json.JSONDecodeError:
            log("Error: Database file contains invalid JSON. Unable to load data.")
            return None
        except Exception as e:
            log(f"Error: Failed to read the database file. {e}")
            return None
    else:
        log(f"Database file {filename} does not exist.")
        return {}

def save_database(data, filename):
    """Save the JSON database to a file."""
    try:
        with open(filename, "w") as file:
            json.dump(data, file, indent=4)
        log(f"Database saved to {filename}")
    except Exception as e:
        log(f"Error: Failed to save the database file. {e}")

def initialize_database(filename):
    """Initialize the database file if it does not exist."""
    if not os.path.exists(filename):
        data = {}
        save_database(data, filename)
        return data
    else:
        return load_database(filename)

def log(message):
    """Print message as a JavaScript console log."""
    print(f"<script>console.log({json.dumps(message)});</script>")

def main():
    print("Content-type: text/html\n")
    print("<html><body>")

    # Print the environment variables for debugging
    for key, value in os.environ.items():
        log(f"{key}={value}")

    # Get the directory where the script is located
    script_directory = get_script_directory()
    filename = os.path.join(script_directory, "database.json")
    log(f"Database filename is {filename}")

    # Initialize and load existing data
    data = initialize_database(filename)
    if data is None:
        data = {}
    log(f"Loaded data: {data}")

    # Determine the request method
    method = os.getenv('REQUEST_METHOD', '').upper()

    if method in ['POST', 'DELETE']:
        if method == 'POST':
            # Read the input data from stdin
            content_length = int(os.getenv('CONTENT_LENGTH', 0))
            post_data = sys.stdin.read(content_length)
            log(f"Received POST data: {post_data}")
            form_data = parse_qs(post_data)
        elif method == 'DELETE':
            content_length = int(os.getenv('CONTENT_LENGTH', 0))
            delete_data = sys.stdin.read(content_length)
            log(f"Received DELETE data: {delete_data}")
            form_data = parse_qs(delete_data)
        
        # Debugging: Print the form keys and values
        for key in form_data.keys():
            log(f"form[{key}]={form_data[key]}")

        log(f"form_data={form_data}")

        name = form_data.get('name', [None])[0]
        salad = form_data.get('salad', [None])[0] if method == 'POST' else None

        log(f"method={method}, name={name}, salad={salad}")

        if method == 'POST' and name and salad:
            # Add or update an entry
            data[name] = salad
            save_database(data, filename)
            log(f"Entry added or updated successfully: {name} likes {salad}.")
        elif method == 'DELETE' and name:
            # Attempt to delete an entry
            if name in data:
                del data[name]
                save_database(data, filename)
                log(f"Entry deleted successfully: {name}.")
            else:
                log(f"Entry not found: {name}.")
        else:
            log("Invalid request. Make sure you provide name and salad parameters for adding, or name for deleting.")
    else:
        log("Unsupported request method. Please use POST or DELETE.")
    
    print("</body></html>")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import cgi
import os
import json

def get_script_directory():
    """Return the directory in which the script is located."""
    return os.path.dirname(os.path.realpath(__file__))

def load_database(filename):
    """Load the JSON database from a file."""
    if os.path.exists(filename):
        try:
            with open(filename, "r") as file:
                return json.load(file)
        except json.JSONDecodeError:
            print("Error: Database file contains invalid JSON. Initializing an empty database.")
            return {}
        except Exception as e:
            print(f"Error: Failed to read the database file. {e}")
            return {}
    else:
        return {}

def save_database(data, filename):
    """Save the JSON database to a file."""
    try:
        with open(filename, "w") as file:
            json.dump(data, file, indent=4)
    except Exception as e:
        print(f"Error: Failed to save the database file. {e}")

def main():
    print("Content-type: text/html\n")
    print()

    # Print the QUERY_STRING environment variable for debugging
    query_string = os.getenv('QUERY_STRING', '')
    print(f"DEBUG: QUERY_STRING: {query_string}<br>")

    # Get the directory where the script is located
    script_directory = get_script_directory()
    filename = os.path.join(script_directory, "database.json")

    # Load existing data
    data = load_database(filename)
    print(f"DEBUG: Loaded data: {data}<br>")

    # Simulating CGI environment for demonstration
    form = cgi.FieldStorage(fp=None, headers=None, environ={'REQUEST_METHOD':'GET', 'CONTENT_TYPE':'application/x-www-form-urlencoded', 'QUERY_STRING': query_string})

    action = form.getvalue('action')
    name = form.getvalue('name')
    salad = form.getvalue('salad')
    print(f"DEBUG: action={action}, name={name}, salad={salad}<br>")

    if action == 'add' and name and salad:
        # Add or update an entry
        data[name] = salad
        save_database(data, filename)
        print(f"Entry added or updated successfully: {name} likes {salad}.")
    elif action == 'delete' and name:
        # Attempt to delete an entry
        if name in data:
            del data[name]
            save_database(data, filename)
            print(f"Entry deleted successfully: {name}.")
        else:
            print(f"Entry not found: {name}.")
    else:
        print("Invalid request. Make sure you provide action, name, and salad parameters for adding, or action and name for deleting.")

if __name__ == "__main__":
    main()

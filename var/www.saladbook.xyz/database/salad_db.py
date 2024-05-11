#!/usr/bin/env python3
import cgi
import os
import json

//export QUERY_STRING="action=add&name=John&salad=Caesar"


def load_database(filename):
    """Load the JSON database from a file."""
    if os.path.exists(filename):
        with open(filename, "r") as file:
            return json.load(file)
    else:
        return {}

def save_database(data, filename):
    """Save the JSON database to a file."""
    with open(filename, "w") as file:
        json.dump(data, file, indent=4)

def main():
    print("Content-type: text/html\n")
    print()

    # Load existing data
    filename = "database.json"
    data = load_database(filename)

    # Simulating CGI environment for demonstration
    query_string = os.getenv('QUERY_STRING', '')
    form = cgi.FieldStorage(fp=None, headers=None, environ={'REQUEST_METHOD':'GET', 'CONTENT_TYPE':'application/x-www-form-urlencoded', 'QUERY_STRING': query_string})

    action = form.getvalue('action')
    name = form.getvalue('name')
    salad = form.getvalue('salad')

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
        print("Invalid request.")

if __name__ == "__main__":
    main()

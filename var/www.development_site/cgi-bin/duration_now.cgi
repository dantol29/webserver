#!/usr/bin/env python3

import time
from datetime import datetime

# Define the duration of the counter in seconds
duration = 2

# Print the HTTP header
print("Content-Type: text/html")
print()

# Print the start time
start_time = datetime.now()
print(f"<html><body>")
print(f"<h1>Counter Script</h1>")
print(f"<p>Start time: {start_time}</p>")

# Counter loop
for i in range(duration):
    time.sleep(1)  # Wait for 1 second

# Print the end time
end_time = datetime.now()
print(f"<p>End time: {end_time}</p>")
print("</body></html>")

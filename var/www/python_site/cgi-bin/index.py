#!/usr/bin/env python3
import datetime

print("Content-type: text/html")
print()
print("<html><head><title>Python Site</title></head><body>")
print("<h1>Welcome to Our Python Site!</h1>")
print("<p>This page was generated with Python CGI.</p>")
current_time = datetime.datetime.now()
print(f"<p>The current date and time is: {current_time}.</p>")
print("</body></html>")

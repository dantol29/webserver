#!/usr/bin/env python3
import os

print('HTTP/1.1 200 OK')
print('Content-type: text/html\n\n')
print('<html>')
print('<head>')
print('<title>Hello World - First CGI Program</title>')
print('</head>')
print('<body>')
print('<h2>Insert an inspirational quote here</h2>')

# Printing MetaVariables variables
print('<h3>MetaVariables Variables</h3>')
print('<ul>')
for key, value in os.environ.items():
    print(f'<li>{key}: {value}</li>')
print('</ul>')

print('</body>')
print('</html>')

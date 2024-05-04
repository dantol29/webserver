# Combined Test Plan for mux_CGI.cpp and parser.cpp

# Objective

This document outlines the test programs to test our Webserver

## Setup Requirements

- The server used for testing should be running on localhost at port 8080

---

## mux_CGI.cpp

### Testing: non-blocking

Designed to test the non-blocking nature of the webserver. It sends concurrent GET requests using multithreading.
To create the largefile (which is gitignored): `dd if=/dev/zero of=var/www/development_site/largefile bs=100M count=1`
(see _docs/mux_tester_dd_command.md_ for more information)

## parser.cpp

### Testing: GET requests and parser

Sends GET requests, both valid and invalid in 4 categories Simple, Query, Header, Body

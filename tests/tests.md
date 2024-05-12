# Combined Test Plan for mux_CGI.cpp and parser.cpp

# Objective

This document outlines the test programs to test our Webserver

## Setup Requirements

- The server used for testing should be running on localhost at port 8080

---

## general_tests.py

- To run follow the guide in `general_tests.md`
- Tests:
	1. large file upload
	2. headers bigget than 1KB
	3. headers bigget than 8KB (431 error)
	4. chunked requests
	5. small file upload
	6. multiple file upload in one request

## mux_CGI.cpp

### Testing: non-blocking

Designed to test the non-blocking nature of the webserver.

It sends concurrent GET requests using multithreading.
The largefile is gitignored and create with the Makefile`dd if=/dev/zero of=var/www/development_site/largefile bs=100M count=1`
(see _docs/mux_tester_dd_command.md_ for more information)

- 3 tests for a 200 in the responses of each request
- 1 test for the non blocking nature (3rd request should be served faster than second (largefile))

## parser.cpp

### Testing: GET requests and parser

Sends GET requests, both valid and invalid in 4 categories Simple, Query, Header, Body

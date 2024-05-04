# Combined Test Plan for mux_CGI.cpp and parser.cpp

# Objective

This document outlines the test programs to test our Webserver

## Setup Requirements

- The server used for testing should be running on localhost at port 8080

---

## mux_CGI.cpp

### Testing: non-blocking

Designed to test the non-blocking nature of the webserver. It sends concurrent GET requests using multithreading.

## parser.cpp

### Testing: GET requests and parser

Sends GET requests, both valid and invalid in 4 categories Simple, Query, Header, Body

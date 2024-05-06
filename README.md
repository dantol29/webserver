# Webserv

Single-threaded web server implemented in C++98 for School 42’s core curriculum.

# Introduction

This project is currently in development by [Daniil(dtolmaco)](https://github.com/dantol29), and [Leo (lmangall)](https://github.com/lmangall) and [Stefano (slombard)](https://github.com/552020).

## Usage

1. **Clone** the repository:
   ```bash
   git clone https://github.com/dantol29/webserver.git
   cd webserver
   ```
2. **Compile** the webserver:
   ```bash
   make
   ```
3. **Run** the server:
   ```bash
   ./webserver
   ```
4. **Visit** `http://localhost:8080` or `http://127.0.0.1:8080` in your browser to view the server response.

## Random resources

- https://datatracker.ietf.org/doc/html/rfc2616
- https://man7.org/linux/man-pages/
- https://chatgpt.com/

## Allowed functions

Check /docs/allowed_functions.md

## Naming Conventions

- camelCase for variable names (except for classes which should be PascalCase)
- snake_case for files (except files containing classes)
- PascalCase for files containing classes
- SCREAMING_CAMEL_CASE for constants and macros

## Coding Standards

- Orthodox Canonical Form
- Add norm about non separation of header and implementation of classes in our internal style guide

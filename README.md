# Webserv

Single-threaded web server implemented in C++98(inspired by NGINX).

# Introduction

Developed by [Daniil](https://github.com/dantol29), [Leo](https://github.com/lmangall) and [Stefano](https://github.com/552020).

# Features
- 16 supported [error codes](https://github.com/dantol29/webserver/blob/main/docs/HTTP_codes.md)
- [Chunked transfer encoding](https://github.com/dantol29/webserver/blob/main/docs/chunked_transfer_encoding.md)
- [HTTP request parsing](https://github.com/dantol29/webserver/blob/main/docs/http_parsing/request.md) according to RFC 9112
- [Configuration file](https://github.com/dantol29/webserver/blob/main/docs/config_file/config_file.md) with 13 configurable parameters(LIMIT_CONN, CGI_PATH ...) and unlimited servers
- Non-blocking CGI according to RFC 3875
- Multiple file uploads
- Supports cookies
- Compatible with Chrome and Firefox
- 10,000 requests per second(tested with Siege)
- Server functionality fully covered with automated tests

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

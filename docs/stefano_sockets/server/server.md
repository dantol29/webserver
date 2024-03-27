# Test Web server

## Port 8080

Port 8080 is commonly used as an alternate to port 80 for web traffic, especially in environments where non-root users are not allowed to bind to ports below 1024. It's widely recognized as a default for HTTP development servers, making it a convenient choice for testing and development purposes. It's not mandatory to use this port; it's chosen here for convenience and convention.

## Socket

```cpp
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("In socket");
    exit(EXIT_FAILURE);
  }
```

This line creates a socket in the Internet domain (AF_INET) using TCP (SOCK_STREAM). 0 specifies that the default protocol for TCP is used. If the socket call fails, it prints an error message and exits.

```cpp
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
```

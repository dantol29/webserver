# Sibling Structures to `struct sockaddr_in`

_Addendum to socketaddr_in.md_

While `struct sockaddr_in` is a fundamental structure for specifying endpoint addresses for IPv4 in socket programming, it's part of a family of structures that cater to various network protocols and address families. In the context of the Webserv project, which focuses on building a web server, `struct sockaddr_in` is indeed predominantly used due to its relevance to IPv4 networking.

The other siblings structs are:

## 1. `struct sockaddr`

Header: <sys/socketh.h>

This is a generic descriptor for any kind of socket operation. It acts as a placeholder for address information for socket functions. Most functions that deal with socket addresses require pointers to `struct sockaddr`, which means that specific address structures (like `struct sockaddr_in`) are typically cast to this type when passed to socket functions.

For example the prototype of the bind() function is: `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);` but when we will use it, we will use it like this:

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket

    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // IPv4 Internet protocols
    addr.sin_port = htons(8080); // Port number
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any address
    // Zero out the rest of the struct
    memset(&(addr.sin_zero), 0, 8);

    // Cast struct sockaddr_in to struct sockaddr and pass it to bind()
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
        perror("bind failed");
        return 1;
    }

    // Proceed with listen(), accept(), etc.

    return 0;
}

```

The relevant like here is: `    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)`. The casting happens here: `(struct sockaddr *)&addr`.

`addr` is a pointer to a type `struct sockeaddr_in` which is casted to a pointer to a more generic `struct sockaddr` type.

#### 2. `struct sockaddr_in6`

Header: <netinet/in.h>

This structure is analogous to `struct sockaddr_in` but is used for IPv6 addresses. It includes fields such as `sin6_family` (address family, AF_INET6 for IPv6), `sin6_port` (port number), and `sin6_addr` (IPv6 address). Using `struct sockaddr_in6` is essential for applications designed to support IPv6 connectivity.

```c
struct sockaddr_in6 {
    sa_family_t     sin6_family;   // AF_INET6
    in_port_t       sin6_port;     // port number
    uint32_t        sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address
    uint32_t        sin6_scope_id; // Scope ID
};
```

IPv6 support is not directly explicit mentioned in the subject of webserv.

## 3. `struct sockaddr_un`

Header: <sys/un.h>

This structure is used for local (Unix domain) socket communication within the same machine or file system. It supports IPC (Inter-Process Communication) without using network layer protocols. It includes fields like `sun_family` (always set to AF_UNIX) and `sun_path` (the pathname of the socket).

```c
struct sockaddr_un {
    sa_family_t sun_family;               // AF_UNIX
    char        sun_path[UNIX_PATH_MAX];  // pathname
};
```

For the Webserv project, while struct sockaddr_un is not crucial, as similar outcomes can be achieved with pipes for IPC, it provides a viable option for certain functionalities. This is particularly relevant for executing CGI scripts or other server-side processes where Unix domain sockets might offer more flexibility or efficiency than traditional pipes.

Given that `socketpair()` is permitted in the project, and considering it typically utilizes Unix domain sockets, the project's guidelines implicitly acknowledge potential use cases for these types of sockets.

Pipes offer simplicity and ease of use for linear, one-way communication between processes, but they are unidirectional and can introduce complexity when bidirectional communication is needed. Unix domain sockets, on the other hand, provide bidirectional communication and a broader set of features, such as non-blocking I/O, at the cost of slightly increased complexity in setup and use.

## 4. `struct sockaddr_storage`

This structure is protocol-agnostic and large enough to hold any socket address type (IPv4, IPv6, Unix domain, etc.). It's useful in scenarios where the application needs to handle multiple address families without knowing them in advance. It ensures compatibility and future-proofing for applications dealing with a variety of network protocols.

```c
struct sockaddr_storage {
    sa_family_t  ss_family;     // address family

    // Implementations should ensure that `struct sockaddr_storage` is
    // large enough to contain any type of socket address (IPv4, IPv6, or Unix domain).
};
```

Probably no use for it in the webserv project.

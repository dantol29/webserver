# struct sockaddress_in

The struct sockaddr_in is a structure defined in the <netinet/in.h> header file in POSIX-compliant systems. It is used to specify an endpoint address to which to connect a socket, particularly for IPv4 addresses. This structure is used when setting up connections using the Internet Protocol (IP).

There are other siblings structs defined in different networking header files:

- sockaddr: the generic one
- sockaddr_in: for IPv4 addresses
- sockaddr_in6: for IPv6 addresses
- sockaddr_un: for 'local sockets' aka UNIX domain sockets
- sockaddr_storage: for protocol agnostic code

For a more detailed exploration of these structures and their applications, please refer to the `sockaddr_family.md` document.

The struct is formed like this:

```c
struct sockaddr_in {
    short            sin_family;   // Address family (AF_INET for IPv4)
    unsigned short   sin_port;     // Port number in Network Byte Order
    struct in_addr   sin_addr;     // IP address in Network Byte Order
    char             sin_zero[8];  // Padding to make structure the same size as struct sockaddr
};

```

## sin_family

**sin_family**: when used within sockaddr_in, the value apparently will be always AF_INET. 'sin_family' is a member of others sockaddr structure, where it can takes multiple values.

_short_: 16 bits int.

## sin_port

This is the port number (in network byte order, which is big-endian) for the connection. Port numbers are used to distinguish between different services running on the same host. The function **htons()** (host to network short) is often used to ensure the port number is in the correct byte order.

Some computer stores the bytes (not the bits!) from left to right, other from right to left. So the 4 byte of an int could be saved as ABCD or DCBA.

## sin_addr

This field contains the IP address for the socket. The struct in_addr is another structure containing a single field s_addr, which is the actual IPv4 address in network byte order. The function inet_pton() or inet_addr() can be used to convert an IPv4 address in dotted decimal notation (e.g., "127.0.0.1") into binary form suitable for assignment to this field.

# sin_zero

This is a padding field, used to make the size of struct sockaddr_in equal to that of struct sockaddr, which is a more general structure used for socket API functions like bind(), connect(), and accept(). The sin_zero field should be set to all zeros using a function like memset() before using the structure. This field is not used for any functional purpose; its presence is solely for compatibility with the general socket address structure.

## Usage

The `struct sockaddr_in` is used in several system calls and functions that deal with network connections in the socket API, not just `bind()` and `connect()`. Here's a rundown of some of the primary ones:

### 1. **socket()**

While `socket()` does not directly take a `struct sockaddr_in`, the socket created is often subsequently configured with `struct sockaddr_in` for network operations. `socket()` creates a new socket of a certain type, identified by an integer file descriptor, for network communication.

A socket is basically something like the end of a pipe. Normally the other end, the other socket, will be on the device on the other side of the network. Contrary to the ends of a pipe, sockets are bidirectional.

### 2. **bind()**

By using bind(), an application informs the operating system's network stack that it wants to "claim" a specific IP address and port number combination. Any incoming network messages or data packets addressed to this specific IP and port should then be directed to the socket associated with that application or service. After binding the socket to a local address and a port, a 'server' will start to listen and accept an evenutal request, wether a client will connect to a server (bind is optional in this case).

This function binds a socket to a local address and port number, specifying where on the network the socket should receive data. It takes a pointer to `struct sockaddr`, but `struct sockaddr_in` is commonly used for IPv4 addresses and cast to `struct sockaddr` in the function call.

A **local address** refers to an IP address assigned to one of the host machine's network interfaces, such as Ethernet or Wi-Fi, which identifies it within a network. This contrasts with remote or non-local addresses, which refer to IP addresses outside the host machine, used to identify other devices on the network or Internet. By binding a socket to a local address, an application specifies its intent to receive data sent to this address.

For applications or services aiming to be accessible across i**multiple network interfaces** (e.g., Ethernet and Wi-Fi), binding a socket to the special "any IP address" (0.0.0.0 for IPv4 or :: for IPv6) allows the socket to listen on the specified port across all network interfaces. This approach eliminates the need for multiple bindings to different IP addresses for each interface, simplifying scenarios where a device, such as a laptop, might switch between different network connections (Ethernet, Wi-Fi, etc.). The application becomes accessible on any network interface of the device, providing flexibility and ease of access without the complexity of managing separate sockets for each interface.

```c
bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### 3. **connect()**

Used by a client to establish a connection to a server, `connect()` takes a socket file descriptor and an address (the server's address) to connect to, again using `struct sockaddr_in` cast to `struct sockaddr`.

```c
connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

### 4. **listen()**

While `listen()` itself doesn't take `struct sockaddr_in` directly, it's used after `bind()` to mark the socket as a passive socket that will be used to accept incoming connection requests, which are specified through `bind()` with a `struct sockaddr_in`.

```c
listen(int sockfd, int backlog);
```

### 5. **accept()**

This function is used by servers to accept a connection request from a client. It can fill in a `struct sockaddr` (typically a `struct sockaddr_in` for IPv4) with the address of the connecting client.

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

### 6. **send() and recv()**

These functions are used for sending and receiving data in a connection-oriented manner, primarily with protocols like TCP. Although they can work with `struct sockaddr_in` for specifying addresses when establishing connections, their primary use in this context does not directly involve addressing as it's handled during the connection setup phase.

```c
send(int sockfd, const void *msg, size_t len, int flags);

recv(int sockfd, void *buf, size_t len, int flags);
```

Note: The `struct sockaddr_in` is also applicable when using `sendto()` and `recvfrom()` functions for specifying the destination or source address. However, in this project, `sendto()` and `recvfrom()` are not permitted as they are utilized for connectionless communication (UDP), which does not align with the project's requirements.

### 7. **getsockname()**

This function retrieves the current address to which a socket is bound, using `struct sockaddr_in` to store the address.

```c
getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

This system call facilitates an aspect of network communication such as querying socket information, with `struct sockaddr_in` playing a crucial role in managing address information for IPv4 networking.

Given the constraints of the Webserv project, where the goal is to implement a web server supporting HTTP communication, the `getpeername()` function is not allowed. The project's allowed functions and system calls focus on establishing connections, data transmission, and server-side socket management without the necessity for `getpeername()`. This function, which retrieves the address of the peer connected to a socket, is not required as the server's implementation within the project guidelines is designed to manage client connections and data flow without needing to directly query the client's address after the connection is established. This approach aligns with the development of a web server that focuses on efficiently handling HTTP requests and responses, ensuring the server remains non-blocking and performs optimally under the specified C++ 98 standard.

Your summary of the usage of `struct sockaddr_in` across various network programming functions is quite comprehensive. It accurately describes the primary ways in which this structure is utilized in the context of socket programming, particularly for IPv4 network communication. You've covered the main functions (`socket()`, `bind()`, `connect()`, `listen()`, `accept()`, `send()`, `recv()`, and `getsockname()`) that are fundamental to establishing connections, transmitting data, and managing sockets.

There are a couple of other functions and concepts where `struct sockaddr_in` could play a role, albeit indirectly in some cases, or in specific network programming scenarios that weren't explicitly covered in your summary:

### 8. `getaddrinfo()` and `freeaddrinfo()`

While not directly using `struct sockaddr_in`, `getaddrinfo()` is an important function for modern network programming that simplifies the process of preparing socket addresses. It's used to resolve hostnames into a dynamically allocated linked list of `struct addrinfo` records, which include `struct sockaddr` pointers suitable for use in network communication functions. This is particularly useful for setting up client sockets. `struct sockaddr_in` can be extracted or manipulated from the `addrinfo` structure when dealing with IPv4 addresses.

```c
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

### 9. `select()`, `poll()`, and `epoll()`

These functions are used for monitoring multiple file descriptors to see if any of them can perform I/O operations (like read or write) without blocking. While they don't use `struct sockaddr_in` directly, they are crucial in the context of servers that handle multiple connections simultaneously. Efficient use of these functions often involves sockets that have been set up and bound to local addresses using `struct sockaddr_in` (or other similar structs) and which are indentfied by file descriptors.

### 10. `setsockopt()` and `getsockopt()`

These functions are used to set and get options on sockets, respectively. Options can affect the behavior of the socket, such as enabling broadcast messages on a datagram socket or setting the socket to be non-blocking. While these functions don't deal directly with `struct sockaddr_in`, they are often used in conjunction with sockets that are configured using this structure to fine-tune their behavior.

```c
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
```

### 11. Direct Socket Address Manipulation

In some advanced scenarios, programmers might directly manipulate the `struct sockaddr_in` to perform operations like creating custom multicast groups (by setting the IP address to a multicast address) or binding a socket to a specific interface using `setsockopt()` with the `SO_BINDTODEVICE` option.

In conclusion, while your document thoroughly addresses the fundamental use cases of `struct sockaddr_in` in network programming, these additional functions and scenarios also benefit from understanding and utilizing this structure. They underscore the versatility and centrality of `struct sockaddr_in` in network programming for IPv4 communications.

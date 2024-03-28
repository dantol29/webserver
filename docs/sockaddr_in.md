# struct sockaddress_in

The struct sockaddr_in is a structure defined in the <netinet/in.h> header file in POSIX-compliant systems. It is used to specify an endpoint address to which to connect a socket, particularly for IPv4 addresses. This structure is used when setting up connections using the Internet Protocol (IP).

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

We have also:

- sockaddr: the generic one
- sockaddr_in: for IPv4 addresses
- sockaddr_in6: for IPv6 addresses
- sockaddr_un: for 'local sockets' aka UNIX domain sockets
- sockaddr_storage: for protocol agnostic code

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

### 7. **getsockname() and getpeername()**

These functions retrieve the current address to which a socket is bound (`getsockname()`) or the address of the peer connected to a socket (`getpeername()`), using `struct sockaddr_in` to store the addresses.

```c
getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

These system calls and functions facilitate various aspects of network communication, such as setting up connections, transmitting data, and querying socket information, with `struct sockaddr_in` playing a crucial role in managing address information for IPv4 networking.

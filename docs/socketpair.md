# socket() and socketpair()

## man socketpair

**NAME**
socketpair – create a pair of connected sockets

**SYNOPSIS**

```c
#include <sys/socket.h>

     int
     socketpair(int domain, int type, int protocol, int socket_vector[2]);
```

**DESCRIPTION**
The socketpair() call creates an unnamed pair of connected sockets in the specified domain domain, of the specified type, and using the optionally specified protocol. The descriptors used in referencing the new sockets are returned in socket_vector[0] and socket_vector[1]. The two sockets
are indistinguishable.

**RETURN VALUES**
The socketpair() function returns the value 0 if successful; otherwise the value -1 is returned and the global variable errno is set to indicate the error.

##

- It is used for IPC.

### Sockets vs pipes

A socket is like a pipe but there are some differencies.

- Similarities:

1. Both sockets and pipes provide a mechanism for **IPC**, allowing different processes to exchange data.

2. Sockets and pipe are both **bidirectional**, i.e. they can used by both processes to read and write to the pipe or to the socket.

3. The data transmission is **buffered**, i.e. it doesn't need to be read the very same moment when it is written.

- Differencies:

1. Pipes are created using `pipe()`, which is more basic and strightforward. Sockets are created using `socketpair()` or `socket()` and require more setup.

2. **Communication Domain** Sockets can be used for both local (UNIX domain sockets) and network (TCP/IP sockets) communication. Pipes are strictly for local IPC.

3. **Type of communication** Sockets can either sream-oriented(TCP-like `SOCK_STREAM`) of message-oriented (UDP-like `SOCK_DGRAM`). Pipes are inherently stream oriented, they treats data as a continous stream of information.

4. **Connection semantics** UNIX domain sockets, can provide connection-oriented sematics with `SOCK_STREAM` ensuring ordered and reliable delivery of data.

## socket() vs socketpair()

The **socket()** function is used to create a single socket, for network or local IPC. To perform network IPC a socket is needed on both machines in the network.

`int socket(int domain, int type, int protocol);`

Parameters:

- _domain_: specifies the communication domain (e.g., AF_INET for IPv4, AF_INET6 for IPv6, AF_UNIX for local UNIX domain sockets)
- _type_: type of the socket (e.g., SOCK_STREAM for TCP, SOCK_DGRAM for UDP).
- _protocol_: Specifies a particular protocol to be used with the socket. Normally, only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as 0.

The **socketpair()** function is specifically designed to create a pair of connected sockets, which are typically used for inter-process communication (IPC) on the same machine. It provides an easy way to set up a bidirectional communication channel between two processes without involving network interfaces.

`int socketpair(int domain, int type, int protocol, int sv[2]);`

Parameters:

- _domain_, _type_ and _protocol_ are same like for `socket()`
- _sv_: An array that will be filled with the file descriptors of the two connected sockets upon successful return.

The sockets created by `socketpair()` are automatically connected to each other. Sockets created by `socket()` need to be explicitely connected to a remote socket (for client-server communication) or bound to a local address (for server sockets).

## Resources

- https://en.wikipedia.org/wiki/Berkeley_sockets

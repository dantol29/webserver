# Allowed Functions

" Everything in C++ 98. execve, dup, dup2, pipe, strerror, gai_strerror, errno, dup, dup2, fork, socketpair, **htons**, htonl, ntohs, ntohl, select, poll, epoll (epoll_create, epoll_ctl, epoll_wait), kqueue (kqueue, kevent), socket, accept, listen, send, recv, chdir bind, connect, getaddrinfo, freeaddrinfo, setsockopt, getsockname, getprotobyname, fcntl, close, read, write, waitpid, kill, signal, access, stat, open, opendir, readdir and closedir."

## Everything in C++98

C++98 and C++ in general doesn't inherently include many system-level functions, but suppor the usage of the C counterpart, since it allows direct usage of C libraries and functions.

## C system call functions

### File and process operations

Process and signals:

- execve
- fork
- waitpid
- kill
- signal (but signation is not listed)

File descriptors:

- dup
- dup2
- pipe
- socketpair [NEW]

File and directories:

- open
- close
- read
- write
- opendir
- readdir
- closedir
- access
- stat [NEW]

C++98 counerparts: C++ has its won file stream libraries (like <fstream>) but these functions provide more direct control.

## Network communication

- socket
- accept
- listen
- send
- recv
- bind
- connect
- getaddrinfo
- freeaddrinfo
- setsocketopt
- getsockname
- getprotobyname

C++98 counerparts: While C++ does not provide direct counterparts in C++98, networking libraries like Boost.ASIO (not allowed for this project) build upon these functions for higher-level networking operations. https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio.html

## Event Polling and Multiplexing

- select
- poll
- epoll
- epoll_create
- epoll_ctl
- epoll_wait
- kqueue
- kqueue
- kevent

These functions are used for multiplexing and event notification mechanisms, crucial for handling multiple connections in a non-blocking manner. C++ does not have built-in support for these mechanisms in the C++98 standard.

## Miscellanous

- strerror
- gai_strerror
- errno

Error handling and reporting functions. C++ uses exceptions for error handling, but these functions provide system-level error information.

### Network byte order conversion functions

- htons
- htonl
- ntohs
- ntohl

They are useful in network programming for ensuring correct byte ordering across different architectures.

- fcntl

File control operations, including setting a file descriptor to non-blocking mode. This is specifically mentioned for MacOS compatibility but has no direct C++ counterpart.

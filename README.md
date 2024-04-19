# webserv

## usage.

1. Compile the webserver running `make` on the root directory of the project
2. Start the webserver with `./webserver`
3. Open a browser and enter `http://localhost:8080` or `http://127.0.0.1:8080`
4. Enjoy the response of the webserv!

## Last resources I have been checking out (Leo's selection):

- I used a cached web archive otherwise you have to pay for Medium
  [HTTP Server: What do you need to know to build a simple HTTP server from scratch](https://web.archive.org/web/20190124234346/https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)

- [GitHub - Kaydooo/Webserv_42](https://github.com/Kaydooo/Webserv_42?tab=readme-ov-file)

- [Webserv: Building a non-blocking web server in C++98, a 42 project](https://m4nnb3ll.medium.com/webserv-building-a-non-blocking-web-server-in-c-98-a-42-project-04c7365e4ec7)

- [42 Seoul GitBook - Webserv](https://42seoul.gitbook.io/webserv)

- [Sockets and Network Programming in C](https://www.codequoi.com/en/sockets-and-network-programming-in-c/)

- [C++ Web Programming](https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm)

- [Building a simple server with C++](https://ncona.com/2019/04/building-a-simple-server-with-cpp/)

## Allowed functions

=> None of theese functionsare inherently C++ functions. They are all from the C standard library, POSIX standard, or system-specific extensions that are primarily used in C programming

| Function/System Call                        | Description                                                                                                                                                                  |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| execve                                      | Executes a program specified by a filename, replacing the current process image with a new one.                                                                              |
| dup                                         | Duplicates an existing file descriptor, creating a copy.                                                                                                                     |
| dup2                                        | Duplicates an existing file descriptor to a specified file descriptor number, possibly overwriting the target.                                                               |
| pipe                                        | Creates a unidirectional data channel that can be used for interprocess communication.                                                                                       |
| strerror                                    | Returns a pointer to a string that describes the error code passed to the function.                                                                                          |
| gai_strerror                                | Converts an error code returned by getaddrinfo into a human-readable string.                                                                                                 |
| errno                                       | A global variable set by system calls and some library functions in the event of an error to indicate what went wrong.                                                       |
| fork                                        | Creates a new process by duplicating the calling process.                                                                                                                    |
| socketpair                                  | Creates a pair of connected, indistinguishable sockets in the UNIX domain.                                                                                                   |
| htons                                       | Converts a 16-bit quantity from host byte order to network byte order.                                                                                                       |
| htonl                                       | Converts a 32-bit quantity from host byte order to network byte order.                                                                                                       |
| ntohs                                       | Converts a 16-bit quantity from network byte order to host byte order.                                                                                                       |
| ntohl                                       | Converts a 32-bit quantity from network byte order to host byte order.                                                                                                       |
| select                                      | Monitors multiple file descriptors, waiting until one or more of the file descriptors become “ready” for some class of I/O operation.                                        |
| poll                                        | Similar to select, but provides a different interface for monitoring multiple file descriptors.                                                                              |
| epoll (epoll_create, epoll_ctl, epoll_wait) | Provides a scalable I/O event notification mechanism. epoll_create creates an epoll instance, epoll_ctl adds/removes file descriptors, and epoll_wait waits for events.      |
| kqueue (kqueue, kevent)                     | An event notification mechanism similar to epoll but specific to BSD operating systems. kqueue creates an instance, and kevent is used to register events and wait for them. |
| socket                                      | Creates a socket that is used for network communications.                                                                                                                    |
| accept                                      | Accepts a connection on a socket, creating a new socket for the connection.                                                                                                  |
| listen                                      | Prepares a socket to accept connection requests, marking it as passive.                                                                                                      |
| send                                        | Sends data on a connection-oriented socket.                                                                                                                                  |
| recv                                        | Receives data from a connection-oriented socket.                                                                                                                             |
| chdir                                       | Changes the current working directory.                                                                                                                                       |
| bind                                        | Associates a socket with a local address.                                                                                                                                    |
| connect                                     | Initiates a connection on a socket.                                                                                                                                          |
| getaddrinfo                                 | Provides network address and service translation in a protocol-independent manner.                                                                                           |
| freeaddrinfo                                | Frees the memory allocated by getaddrinfo.                                                                                                                                   |
| setsockopt                                  | Sets options on a socket.                                                                                                                                                    |
| getsockname                                 | Gets the current address to which a socket is bound.                                                                                                                         |
| getprotobyname                              | Maps a protocol name to a protocol number.                                                                                                                                   |
| fcntl                                       | Manipulates file descriptor options.                                                                                                                                         |
| close                                       | Closes a file descriptor, so it no longer refers to any file and may be reused.                                                                                              |
| read                                        | Reads data from a file descriptor into a buffer.                                                                                                                             |
| write                                       | Writes data from a buffer to a file descriptor.                                                                                                                              |
| waitpid                                     | Waits for state changes in a child of the calling process.                                                                                                                   |
| kill                                        | Sends a signal to a process or a group of processes.                                                                                                                         |
| signal                                      | Sets a function to handle a signal.                                                                                                                                          |
| access                                      | Checks the calling process’s permissions for a file.                                                                                                                         |
| stat                                        | Retrieves information about a file.                                                                                                                                          |
| open                                        | Opens a file, returning a file descriptor for the file.                                                                                                                      |
| opendir                                     | Opens a directory stream corresponding to a directory name.                                                                                                                  |
| readdir                                     | Reads a directory entry from a directory stream.                                                                                                                             |
| closedir                                    | Closes a directory stream.                                                                                                                                                   |

## Naming Conventions

- camelCase for variable names (except for classes which should be PascalCase)
- snake_case for files (except files containing classes)
- PascalCase for files containing classes
- SCREAMING_CAMEL_CASE for constants and macros

## Coding Standards

- Orthodox Canonical Form
- Add norm about non separation of header and implementation of classes in our internal style guide

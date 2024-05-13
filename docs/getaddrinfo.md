# Using `getaddrinfo` to Prepare ServerSocket for Binding

We are using the `getaddrinfo` function in the prepareServerSocket method of the ServerSocket object. Before binding the socket to an IP:port combinatio we need to 'prepare' it.

The ServerSocket class contains:

- a Listen object, which contains information relative to the listen directive in the server block in the configuration file.
- a file descriptor that we get from the socket() fuction
- a serverSocketAddr variable, which is of type sockaddr_storage. This is an agnostic type that could host IPv4 and IPv6 addresses.

...

The `getaddrinfo` function is an essential tool in network programming, especially when setting up server sockets that need to listen on specific ports and IP addresses. This guide provides an overview of how to use `getaddrinfo` for preparing a `ServerSocket` address and the differences between `getaddrinfo` and `inet_ntop`.

Certainly! Let's continue the exposition in the detailed and informative style you've started:

---

# Using `getaddrinfo` to Prepare ServerSocket for Binding

We utilize the `getaddrinfo` function within the `prepareServerSocket` method of the `ServerSocket` object to prepare the socket for binding to an IP:port combination. This preparation is crucial for ensuring the server is ready to accept connections on the specified addresses and ports as configured in the server blocks of the configuration file.

## Composition of the ServerSocket Class

The `ServerSocket` class is designed to encapsulate all necessary information and functionality for handling server sockets efficiently and robustly. It includes:

- **Listen Object**: Contains information about the listening directive from the server's configuration block, including the IP address and port number on which the server should listen. This object also indicates whether the address is IPv4 or IPv6, guiding how the address setup should be handled.
- **File Descriptor**: Obtained from the `socket()` function, this descriptor represents the socket in the system. Operations on the socket, such as binding and listening, will use this descriptor.

- **serverSocketAddr Variable**: A `sockaddr_storage` type variable that can store either IPv4 or IPv6 addresses. `sockaddr_storage` is protocol-agnostic and large enough to accommodate all types of socket addresses used in the system.

## Preparing the ServerSocket for Binding

The `prepareServerSocket` method is responsible for setting up the `serverSocketAddr` structure with the appropriate address and port information before the socket is bound. Here’s how the method works step-by-step:

1. **Clearing and Setting Hints**:

   - The method starts by clearing the `struct addrinfo hints` to ensure it's initialized to zeros.
   - The hints are set to specify the socket type (`SOCK_STREAM` for TCP) and the appropriate address family based on the Listen object (IPv4 for `AF_INET`, IPv6 for `AF_INET6`). The `AI_PASSIVE` flag is used to indicate the socket address will be used in a bind operation, which should use wildcard IP addresses if no specific IP is provided.

2. **Handling IP Address Specificity**:

   - If the Listen object specifies "any" or is empty, it indicates that the server should listen on all available network interfaces. In such cases, `NULL` is passed as the node argument to `getaddrinfo`, which, combined with `AI_PASSIVE`, results in using wildcard addresses.
   - If a specific IP address is provided, it is passed to `getaddrinfo` to resolve or convert into a binary format suitable for network operations.

3. **Calling getaddrinfo**:

   - `getaddrinfo` is invoked with the IP address (or `NULL` for wildcard), the port number as a string, and the prepared hints. It resolves or prepares the address and fills the `addrinfo` structure pointed to by its result pointer.

4. **Copying the Result**:

   - The address information from the first `addrinfo` result is copied into `serverSocketAddr` using `memcpy`. This ensures that the socket address is ready for the bind operation.

5. **Resource Cleanup**:
   - After copying the address data, the dynamically allocated `addrinfo` linked list is freed using `freeaddrinfo` to prevent memory leaks.

## Example Code Snippet

```cpp
void ServerSocket::prepareAddress() {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = _listen._isIpv6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const char* ip = (_listen._ip.empty() || _listen._ip == "any") ? NULL : _listen._ip.c_str();
    std::string port = std::to_string(_listen._port);

    if (getaddrinfo(ip, port.c_str(), &hints, &result) == 0) {
        memcpy(&_serverSocketAddr, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
    }
}
```

## Conclusion

The `prepareServerSocket` method effectively encapsulates the complexity of setting up socket addresses, making the `ServerSocket` class robust and easy to integrate into server applications. By leveraging `getaddrinfo`, the class can handle both IPv4 and IPv6 configurations seamlessly and is prepared for future expansions in network technology or configurations.

## Overview of `getaddrinfo`

`getaddrinfo` is a system function that provides a way to resolve DNS names into socket addresses, or prepare socket addresses in a protocol-agnostic manner. This function is particularly useful in setting up server sockets because it can handle both IPv4 and IPv6 addresses and does not require manual handling of different socket address structures.

### Syntax

```c
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```

- **node**: The host name or IP address.
- **service**: The service name or port number.
- **hints**: Hints about the type of service.
- **res**: Pointer to the linked-list of result addresses.

## Preparing `ServerSocket` for Binding

To prepare a `ServerSocket` for binding, you typically need to set up its address structure (`sockaddr_storage` in this case) that will be used in the `bind()` call. `getaddrinfo` can automatically fill this structure based on the provided inputs.

### Example Usage

Here's how you might use `getaddrinfo` in a function to prepare the `ServerSocket` address:

```cpp
void ServerSocket::prepareAddress() {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // For wildcard IP address

    const char* ip = _listen._ip.empty() || _listen._ip == "any" ? NULL : _listen._ip.c_str();
    std::string port = std::to_string(_listen._port);

    if (getaddrinfo(ip, port.c_str(), &hints, &result) == 0) {
        memcpy(&_serverSocketAddr, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
    }
}
```

## Differences from `inet_pton` and `inet_ntop`

`inet_pton` (Presentation to Network) and `inet_ntop` (Network to Presentation) are functions used to convert IP addresses between their string (presentation) form and their binary (network) form in a specific IP family's socket address structure.

- **`inet_pton`**: Converts IP addresses from text to binary form.
- **`inet_ntop`**: Converts IP addresses from binary to text form.

While `getaddrinfo` is used for preparing complete socket addresses (including resolving DNS names or setting up port numbers), `inet_pton` and `inet_ntop` are solely focused on converting the IP address part of the socket setup.

## Conclusion

Preparing a socket for binding involves setting up its address structure correctly for the `bind()` system call. Using `getaddrinfo` simplifies this process by handling various complexities associated with different protocols and address types. It is more robust and versatile compared to manually configuring socket addresses or using simpler conversion functions like `inet_pton` and `inet_ntop`.

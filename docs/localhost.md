# localhost aka 127.0.0.1/8

`http://127.0.0.1:8080` or `http://localhost:8080`

The IP Address: 127.0.0.1 is the loopback Internet Protocol (IP) address also used to refer to your own computer. It’s part of the loopback network (127.0.0.0/8 IP block), which is reserved by the Internet Engineering Task Force (IETF) for loopback purposes. Any IP address starting with 127. is loopback, but 127.0.0.1 is the most commonly used.

What happens when an http request is sent to localhost:

1. When a web browser prepares to send an HTTP request to 127.0.0.1 (or any other address) the first step involves opening a network socket.

2. After the socket is opened, the HTTP request is sent through this socket directed to 127.0.0.1. This address is recognized by the **network stack** as the **loopback address**, which is used for sending data to the same host.

_Loopback Interface_: The loopback interface is a virtual network interface implemented in the network stack. It's special in that it doesn't correspond to any physical network hardware. Instead, its primary purpose is for internal testing and communication within the host.

Network Stack's Role: When data is sent to the loopback address, the network stack routes this data internally. It bypasses the physical network interfaces entirely and redirects the data back to the host itself. This happens at the IP layer of the network stack, which recognizes the loopback address range (127.0.0.0/8) and handles it specially.

The _IP layer_ identifies that the destination address is within the loopback range and thus forgoes the usual process of determining which network interface to use for sending the data out.
The _transport layer (TCP or UDP)_, TCP in our case, then manages the connection, ensuring that data sent to the loopback address is directed to the correct port (8080 in our stub webserv) on the local machine. If a service is listening on the specified port, it receives the data as if it had come from an external source.

## Reserved IP Adresses ranges

The Internet Engineering Task Force (IETF) has reserved several IP address blocks for various testing, documentation, and educational purposes, beyond just the loopback address range (`127.0.0.0/8`).

- **Loopback Addresses (`127.0.0.0/8`)**: This block contains 16,777,214 addresses (`127.0.0.1` to `127.255.255.254`) used for loopback purposes, allowing a device to send messages to itself.

- **Private Networks (`10.0.0.0/8`, `172.16.0.0/12`, `192.168.0.0/16`)**: These ranges are set aside for use in private networks. They can be used by anyone setting up internal networks and are not routed on the global internet.

- **Link-Local Addresses (`169.254.0.0/16`)**: These are used for automatic IP address assignment on a local network when no external DHCP server is available.
- **Test-Net Addresses (`192.0.2.0/24`, `198.51.100.0/24`, `203.0.113.0/24`)**: Designated for use in documentation and network examples to avoid conflicts with real internet addresses.
- **Multicast Addresses (`224.0.0.0/4`)**: Reserved for multicast groups (used to send a single packet to multiple destinations).
- **Future Use and Research (`240.0.0.0/4`)**: This block was originally reserved for future use, though parts of it are now being reconsidered for various purposes, including research.

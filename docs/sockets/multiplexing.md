# Multiplexing

Multiplexing is a stratedy to let the same medium to carry different messages together, like having different letters in the same envelope, or on the same sheet of paper.

> In telecommunications and computer networking, multiplexing (sometimes contracted to muxing) is a method by which multiple analog or digital signals are combined into one signal over a shared medium. The aim is to share a scarce resource – a physical transmission medium.[citation needed] For example, in telecommunications, several telephone calls may be carried using one wire. Multiplexing originated in telegraphy in the 1870s, and is now widely applied in communications. In telephony, George Owen Squier is credited with the development of telephone carrier multiplexing in 1910.

But in computing it has a similar but analogous meaning:

> In computing, I/O multiplexing can also be used to refer to the concept of processing multiple input/output events from a single event loop, with system calls like poll and select (Unix).

In this scenario we are leveraging an event loop to process multiple and different input/output with a single-threaded process.

Types

When a socket is marked as "readable" during a `poll` (or `select`, `epoll`, etc.) operation, it signifies that there is data available to be read without blocking, or that a certain network event has occurred. The meaning can vary depending on the context—whether it's a server listening for incoming connections or a socket that's connected to a peer. Let's clarify what "readable" means in these contexts:

### For a Server Socket

- **Incoming Connection**: For a server socket (one that's been set up to listen for incoming connections), "readable" means that an incoming connection request has been received. This doesn't involve actual "data" in the sense of application-level messages but rather indicates that the low-level TCP three-way handshake has been completed by the operating system's networking stack. The server application can now call `accept` to accept the incoming connection, which will return a new socket file descriptor for the newly established connection. This new socket is what the server uses to communicate with the connected client.

### For a Connected Socket

- **Data Available**: For a socket that's connected to a peer (either a client connected to a server or a server's socket connected to a client), "readable" means that there is incoming data available to read. This data is whatever the peer has sent: it could be an HTTP request, a message in a chat application, or any other data transmitted over the network.
- **Connection Closed**: Additionally, a socket being readable can also indicate that the other side of the connection has closed the socket. In this case, a read operation will return `0`, indicating an orderly shutdown by the peer.

- **Error or Urgent Data**: In some cases, readability can also indicate an error condition on the socket or the presence of out-of-band data (urgent data), but these are less common scenarios.

### Who Writes the Data?

The data that makes a connected socket readable is written by the application on the other end of the connection. For example, if your server is connected to a web browser client, the data could be HTTP requests sent by the browser. The operating system's network stack handles the transmission of this data across the network, buffering it until your application reads it from the socket.

In the case of an incoming connection making a listening socket readable, the "data" is the result of the TCP/IP protocol's internal mechanisms, specifically the three-way handshake that establishes a new connection. This isn't application-level data but rather signaling at the protocol level, managed by the OS.

Understanding the context in which a socket becomes readable is crucial for correctly interpreting the event and responding appropriately, whether by accepting a new connection or reading incoming data.

## Resources

- (Multiplexing, Wikipedia)[https://en.wikipedia.org/wiki/Multiplexing]

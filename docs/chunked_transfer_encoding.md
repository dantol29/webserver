# Chunked transfer encoding

Chunked transfer encoding is a mechanism in HTTP/1.1 for sending HTTP payloads in a series of "chunks" without needing to know the entire payload size beforehand. This feature is particularly useful for dynamically generated content where the total content length cannot be easily determined before sending the HTTP header. Chunked transfer encoding allows a server to start sending a response's body to the client as soon as the first part of the body is ready, improving the responsiveness and efficiency of web communications.

### Key Points of Chunked Transfer Encoding

- **Specified in HTTP/1.1**: Chunked transfer encoding is defined in the HTTP/1.1 specification (RFC 2616 and updated by RFC 7230). It's not available in HTTP/1.0.

- **Transfer of Dynamically Generated Content**: It is especially useful for sending files of unknown size at the time the header is sent, such as dynamically generated content or streaming data.

- **Format**: The data is sent in a series of chunks. Each chunk is preceded by its size in bytes, represented as a hexadecimal number followed by `\r\n` (carriage return and line feed). The chunk itself follows this line, and then another `\r\n` sequence concludes the chunk. The end of the message is signaled by a zero-length chunk (`0\r\n\r\n`), optionally followed by trailers (additional headers) that are also terminated by `\r\n`.

- **Advantages**: This encoding enables the sender to start transmitting data before it's all ready, allowing for more efficient use of resources and faster perceived load times for large or dynamically generated resources.

- **Headers**: When a message is sent with chunked transfer encoding, the `Transfer-Encoding: chunked` header is included in the response, and the `Content-Length` header is omitted because the content length is not known in advance.

### Example

Here's a simplified example of how chunked transfer encoding might look in an HTTP response:

```
HTTP/1.1 200 OK
Content-Type: text/plain
Transfer-Encoding: chunked

5
hello
7
world!

0
```

Or, more clearly

```

HTTP/1.1 200 OK
Content-Type: text/plain
Transfer-Encoding: chunked

5\r\n
hello\r\n
7\r\n
world!\r\n
0\r\n
\r\n
```

This response contains the text "hello world!" sent in two chunks: "hello" (5 bytes) and "world!" (7 bytes), followed by a zero-length chunk indicating the end of the response.

### Use in HTTP Requests

While chunked transfer encoding is most commonly associated with HTTP responses, it can also be used in HTTP requests.

### Server-to-Client Examples

1. **Streaming Services**: we know them!

2. **Real-time Data Feeds**: Applications that provide real-time data, such as stock tickers, live sports scores, or social media feeds, often use chunked transfer encoding to push updates to clients as new data comes in, without having to close and re-establish the connection for each update.

3. **Large File Downloads**: When serving large files, servers might opt for chunked transfer encoding to start the transfer immediately without waiting to calculate the entire file size, which can be especially useful for files that are being generated or altered on the fly.

4. **Server-Sent Events (SSE)**: SSE is a standard describing how servers can initiate data transmission towards browser clients once an initial client connection has been established. It's commonly used for sending real-time notifications and updates to web clients, and it leverages chunked transfer encoding to keep the connection open and send events as they happen.

### Client-to-Server Examples

Chunked transfer encoding from the client to the server is less common but is applicable in scenarios where the client generates data in real-time or the total request size is not known in advance:

1. **File Uploads**: A client uploading a large file, especially one generated in real-time (like a video recording), might not know the total file size at the beginning of the upload. Chunked transfer encoding allows the client to start sending data immediately without calculating the total size.

2. **Live Data Collection**: In scenarios involving live data collection, such as telemetry from IoT devices or live user input from web applications, the data is sent to the server as it's generated, without a predefined end.

3. **Streaming APIs**: Some REST or GraphQL APIs support receiving data in chunks from clients, which is useful for processing large datasets or files without requiring the client to hold the entire dataset in memory.

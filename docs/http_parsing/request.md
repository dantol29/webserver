## After a `URL` is entered into the browser, an `HTTP request` is sent

:shipit: _RFC 9112 (https://datatracker.ietf.org/doc/html/rfc9112#name-request-line)_

### Request string has three main components:

- request line (method, URI, and HTTP version)
- headers (content type, content length and more)
- body (html, json, plain, octet-stream)

-------------------------------------------------------------------------------------------------------------------

### REQUEST LINE

- Syntax:  `method token, single space (SP), request-target, SP, protocol version`.
- Example: `GET /index.html HTTP/1.1`.

#### ⚠️ Request line rules

1. Method token is **case-sensitive**.
2. There are **four distinct formats** for the request-target (origin-form, absolute-form, authority-form, asterisk-form).
3. **No whitespace** is allowed in the request-target.
4. A client **MUST** send a Host header field.
5. If the target URI includes an authority component, then a client **MUST** send a field value for Host that is identical to that authority component, excluding any userinfo subcomponent and its "@" delimiter.
6. If the authority component is missing or undefined for the target URI, then a client **MUST** send a Host header field with an empty field value.

| Method  | Description |
| ------------- | ------------- |
| GET  | Retrieves data from the server |
| POST  | Submits data to be processed to the server |
| PUT  | Uploads a resource to the server |
| DELETE  | Requests the server to delete a resource  |
| HEAD  | Similar to GET but requests only the headers |
| CONNECT  | Establish a tunnel to the server |
| OPTIONS  | Request information about the communication options |
| TRACE  | Echo back the received request to the client  |

#### Requst-target formats

1. <b>Origin-form</b>

   - _Example_ `http://www.example.org/where?q=now` => `GET /where?q=now HTTP/1.1`
   - a client **MUST** send only the absolute path and query components of the target URI as the request-target.
   - If the target URI's path component is empty, the client **MUST** send "/" as the path within the origin-form of request-target.

2. <b>Absolute-form</b>

   - _Example_ `http://www.example.org/pub/pro.html` => `GET http://www.example.org/pub/pro.html HTTP/1.1`
   - A client **MUST** send a Host header field
   - When a proxy receives a request with an absolute-form of request-target, the proxy **MUST** ignore the received Host header field and instead replace it with the host information of the request-target. A proxy that forwards such a request MUST generate a new Host field value based on the received request-target rather than forward the received Host field value???????(wtf).
   - The origin server **MUST** ignore the received Host header field and instead use the host information of the request-target. Note that if the request-target does not have an authority component, an empty Host header field will be sent in this case?????????(wtf).

3. <b>Authority-form</b>

   - _Example_ `www.example.com:80` => `CONNECT www.example.com:80 HTTP/1.1`
   - This form of is only used for **CONNECT** requests. It consists of only the uri-host and port number of the tunnel destination, separated by ":".
   - A client **MUST** send only the host and port of the tunnel destination as the request-target
   - Sends the scheme's **default port** if the target URI elides the port

4. <b>Asterisk-form</b>
   - _Example_ `OPTIONS * HTTP/1.1`
   - This form of is only used for **OPTIONS** request
   - When a client wishes to request OPTIONS for the server as a whole, the client **MUST** send only "*" as the request-target (`OPTIONS * HTTP/1.1`)
-------------------------------------------------------------------------------------------------------------------

### HEADERS

...

-------------------------------------------------------------------------------------------------------------------

### BODY

#### ⚠️ Body rules

1. A sender **MUST NOT** send a Content-Length header field in any message that contains a Transfer-Encoding header field.
2. A recipient **MUST** be able to parse the chunked transfer coding because it plays a crucial role in framing messages when the content size is not known in advance.
3. A server **MUST NOT** send a Transfer-Encoding header field in any response with a status code of 1xx (Informational) or 204 (No Content).
4. A server **MUST NOT** send a Transfer-Encoding header field in any 2xx (Successful) response to a CONNECT request.
5. A server that receives a request message with a transfer coding it does not understand **SHOULD** respond with 501 (Not Implemented).
6. The server **MUST** close the connection after responding to such a request to avoid the potential attacks
7. A server that receives an HTTP/1.0 message containing a Transfer-Encoding header field **MUST** treat the message as if the framing is faulty, even if a Content-Length is present.
8. If a message is received without Transfer-Encoding and with an invalid Content-Length header field, then the message framing is invalid and the recipient **MUST** treat it as `400` (unrecoverable error).

-------------------------------------------------------------------------------------------------------------------

### :bomb: Error status codes

- A server that receives a method longer than any that it implements **SHOULD** respond with a `501` (Not Implemented) status code.
- A server that receives a request-target longer than any URI it wishes to parse **MUST** respond with a `414` (URI Too Long) status code.
- An invalid request-line **SHOULD** respond with either a `400` (Bad Request) error or a `301` (Moved Permanently) redirect with the request-target properly encoded.
- A server **MUST** respond with a `400` (Bad Request) status code to any request message that lacks a Host header field and to message that contains more than one Host header field line or a Host header field with an invalid field value.
- If the unrecoverable error is in a request message, the server **MUST** respond with a `400` (Bad Request) status code and then close the connection.
- A server **MAY** reject a request that contains a message body but not a Content-Length by responding with `411` (Length Required).

### :placard: Notes:

- `Finite state machines` (FSMs) can be very helpful in parsing HTTP requests
- Explore `transfer-encoding` more
- `Chunked Transfer Coding` might be mandatory to implement

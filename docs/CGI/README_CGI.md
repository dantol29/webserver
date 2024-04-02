## CGI general informations

CGI (Common Gateway Interface) is a protocol or interface that enables web servers to execute external programs, known as CGI scripts, to generate web content dynamically. This mechanism allows the web server to pass data to and from an application, making it possible to generate and deliver dynamic content to web users.

The process works as follows: when the web server receives a request, it forwards the data to the specified program. It sets various environment variables and marshals the parameters via standard input (stdin) and standard output (stdout).

CGI scripts are conventionally stored in `/cgi-bin/`, and have a `.cgi` extension. 

Documentation for CGI can be found in the RFC 3875 ---(IS THAT THE GOOD RFC ?)--- available at [https://www.ietf.org/rfc/rfc3875.txt](https://www.ietf.org/rfc/rfc3875.txt)

### Server-Side vs. Client-Side Programming


| Feature                                   | Client-side | Server-side |
|-------------------------------------------|:-----------:|:-----------:|
| Enhances interactivity of web pages       |      ✓      |             |
| Relies heavily on server-stored data      |             |      ✓      |
| Requires minimal interaction with the user|             |      ✓      |
| Reduces server load                       |             |      ✓      |
| Involves frequent user interactions       |      ✓      |             |


JavaScript, for example, is typically used for client-side scripting to enhance the interactivity of web pages without the need to constantly communicate with the server.


Regardless of the programming model, the server must distinguish between URLs that point to static HTML files and those that map to scripts requiring execution. 

Before executing a CGI script, the web server sets environment variables based on the data received in the request, such as the client's IP address and the request headers. If the request URL includes a query string (indicated by a `?`), the data following the question mark is stored in its own environment variable.

To return output to the web server, a CGI script writes HTTP headers and the HTML document to stdout using standard output commands in the programming language of choice, such as `print` in Perl or Python, `printf` in C, or `System.out.println` in Java.

For more information on CGI and server-side programming, refer to the tutorial available at [https://www.garshol.priv.no/download/text/http-tut.html](https://www.garshol.priv.no/download/text/http-tut.html).

______________________________________






______________________________________


### CGI's Historical Role in Web Development:

- **Historical Context:** CGI was one of the earliest methods used to generate dynamic web content, serving as a bridge between the web server and software applications running on the server.

- **Modern Use:** While CGI set the foundation for dynamic content on the web, modern web development often uses more contemporary approaches such as server-side scripting frameworks (e.g., Node.js for JavaScript, Flask for Python) and application programming interfaces (APIs) that offer more efficiency, scalability, and ease of development. Nonetheless, CGI's conceptual model underpins the idea of server-side processing and dynamic content generation, influencing current technologies and methodologies.

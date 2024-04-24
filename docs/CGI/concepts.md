## CGI general informations

CGI (Common Gateway Interface) is a protocol or interface that enables web servers to execute external programs, known as CGI scripts, to generate web content dynamically. This mechanism allows the web server to pass data to and from an application, making it possible to generate and deliver dynamic content to web users.

The process works as follows: when the web server receives a request, it forwards the data to the specified program. It sets various MetaVariables variables and marshals the parameters via standard input (stdin) and standard output (stdout).

CGI scripts are conventionally stored in `/cgi-bin/`, and have a `.cgi` extension.

Documentation for CGI can be found in the RFC 3875 available at [https://www.ietf.org/rfc/rfc3875.txt](https://www.ietf.org/rfc/rfc3875.txt)

[RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875)

```plaintext
The server acts as an application gateway.  It receives the request
   from the client, selects a CGI script to handle the request, converts
   the client request to a CGI request, executes the script and converts
   the CGI response into a response for the client.
```

### Server-Side vs. Client-Side Programming

| Feature                                    | Client-side | Server-side |
| ------------------------------------------ | :---------: | :---------: |
| Enhances interactivity of web pages        |      ✓      |             |
| Relies heavily on server-stored data       |             |      ✓      |
| Requires minimal interaction with the user |             |      ✓      |
| Reduces server load                        |             |      ✓      |
| Involves frequent user interactions        |      ✓      |             |

JavaScript, for example, is typically used for client-side scripting to enhance the interactivity of web pages without the need to constantly communicate with the server.

Regardless of the programming model, the server must distinguish between URLs that point to static HTML files and those that map to scripts requiring execution.

Before executing a CGI script, the web server sets MetaVariables variables based on the data received in the request, such as the client's IP address and the request headers. If the request URL includes a query string (indicated by a `?`), the data following the question mark is stored in its own MetaVariables variable.

To return output to the web server, a CGI script writes HTTP headers and the HTML document to stdout using standard output commands in the programming language of choice, such as `print` in Perl or Python, `printf` in C, or `System.out.println` in Java.

For more information on CGI and server-side programming, refer to the tutorial available at [https://www.garshol.priv.no/download/text/http-tut.html](https://www.garshol.priv.no/download/text/http-tut.html).

### Script vs program:

In a nutshell: scripts are interpreted at runtime, programs are compiled

- **Script**:
  - A script is usually a file containing a set of commands that are executed by an interpreter.
  - In the context of CGI, a script is often written in scripting languages like Perl, Python, or PHP.
  - Scripts are typically not compiled but are interpreted at runtime
  - Scripts are often used for automating tasks, processing data, generating web page content dynamically, and handling form data on web servers.
- **Program**:
  - A program refers to a compiled executable that is run directly by the operating system.
  - In the web server context, a program might serve a similar purpose to a CGI script (e.g., generating dynamic content) but is executed as a standalone application.
  - Programs are generally faster than scripts because they are pre-compiled into machine code, which the CPU can execute directly without the need for an interpreter.
  - The compilation step adds an additional layer of complexity to development and deployment but can optimize performance and efficiency for computationally intensive tasks.

The distinction between scripts and programs has become less clear-cut with the evolution of scripting languages. For example, Python and JavaScript can be used to write simple scripts as well as complex, full-fledged applications.

CGI programs can be written in almost any programming language, including compiled languages like C, C++, or Go, as well as interpreted scripting languages like Perl, Python, or Ruby.

### CGI's Historical Role in Web Development:

- **Historical Context:** CGI was one of the earliest methods used to generate dynamic web content, serving as a bridge between the web server and software applications running on the server.

- **Modern Use:** While CGI set the foundation for dynamic content on the web, modern web development often uses more contemporary approaches such as server-side scripting frameworks (e.g., Node.js for JavaScript, Flask for Python) and application programming interfaces (APIs) that offer more efficiency, scalability, and ease of development. Nonetheless, CGI's conceptual model underpins the idea of server-side processing and dynamic content generation, influencing current technologies and methodologies.

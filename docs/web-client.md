# web client

A web client is any application or software able to send an http request.
Not every client in a client-server relationship is a web client, think of Minitalk, and not only web client is a web browser. A web browser is only one of the possible web clients.

## List of web clients programs

### Web browsers

We already know them, no need of explanations.

### Command-Line Tools

There are several command line tools able to do http requests, like `curl` and `wget`, or `telnet`

**curl**: curl is a tool for transferring data from or to a server.

Try `curl http://google.com` or `https://www.google.com`

With curl we can perform not only GET requests but also POST requests.

**wget** is used to download files


- `wget http://example.com/somefile.zip`

**telnet** is mentioned in the subject.

### Http Clients in Programming Languages

Basically most of the program languages have libraries or built-in support for making HTTP requests.

Certainly! Let's dive into the most commonly used HTTP client libraries or interfaces in C++, C, Python, PHP, and JavaScript, as well as how frameworks like React and Next.js perform HTTPS requests.

#### C++

In C++, while the standard library doesn't directly provide an HTTP client, there are several popular libraries used to make HTTP requests:

- **C++ REST SDK (also known as Casablanca)**: Offers a modern asynchronous API for cloud-based client-server communication, with support for RESTful services.
- **libcurl / cURLpp**: `libcurl` is a versatile library written in C for transferring data with URL syntax, supporting numerous protocols including HTTP and HTTPS. `cURLpp` is a C++ wrapper around `libcurl`.
- **Boost.Beast (part of Boost libraries)**: Designed for working with HTTP and WebSocket protocols. It's built on top of Boost.Asio and provides a synchronous and asynchronous API.

#### C

In C, the most direct way to perform HTTP requests is using the `libcurl` library. It's widely supported, mature, and capable of handling various protocols, making it the de facto standard for HTTP requests in C.

- **libcurl**: Provides a C-based library (libcurl) for transferring data with URL syntax. It's versatile and widely used in C applications for HTTP/HTTPS requests.

#### Python

Python has both built-in libraries and external packages for handling HTTP requests:

- **requests**: An elegant and simple HTTP library for Python, built for human beings. It's the most popular choice for making HTTP requests in Python.
- **http.client (formerly httplib)**: A module in Python's standard library for HTTP and HTTPS client communication. It's lower-level than `requests`.
- **urllib**: Another module in Python's standard library used for fetching URLs. It provides a slightly higher-level interface than `http.client`.

#### PHP

PHP includes built-in functions and libraries for making HTTP requests, with the most common being:

- **cURL**: PHP supports libcurl, allowing it to connect and communicate with different types of servers using different types of protocols.
- **file_get_contents() / stream_context_create()**: These functions can be used for making HTTP GET requests. `stream_context_create()` can also be configured for HTTP POST requests and more.

#### JavaScript

In the JavaScript ecosystem, there are both browser-based and Node.js-based solutions:

- **Fetch API**: A modern interface in the browser for making asynchronous HTTP requests. It's built into modern web browsers.
- **XMLHttpRequest (XHR)**: An older way of making HTTP requests in browsers, but still in use for backward compatibility.
- **axios**: A promise-based HTTP client for the browser and Node.js. It's popular due to its wide browser compatibility and features.
- **Node.js `http` and `https` modules**: Built-in modules in Node.js for HTTP and HTTPS requests, respectively. They are used for server-side requests.

#### Frameworks: React and Next.js

- **React** itself doesn't include built-in ways to make HTTP requests as it's a library for building user interfaces. However, React applications often use `fetch`, `axios`, or other libraries for such tasks. React components will use these libraries in their lifecycle methods or hooks to request data.

- **Next.js** In Next.js, you can use any JavaScript HTTP client library to make requests. For server-side data fetching, Next.js provides specific functions like `getServerSideProps` or `getStaticProps`, where you can use `fetch`, `axios`, or any other promise-based HTTP client to fetch data during the rendering process on the server. On the client side, within React components, you can use `fetch`, `axios`, or any other method just like in a regular React app.

Both React and Next.js rely on the ability to use JavaScript libraries or the Fetch API for making HTTP/S requests, rather than having a dedicated built-in mechanism. This approach provides flexibility to developers to choose the library that best fits their needs.

### API Testing Tools

Software designed for testing APIs can also act as HTTP clients. Examples include Postman, Insomnia, and SoapUI. These tools offer a graphical interface for sending requests to URLs and viewing responses.

**Postman** is widely used to test APIs.

### Mobile and Desktop Apps

Many apps that require data from the internet (such as social media apps, news aggregators, or cloud-based services) make HTTP requests to servers to retrieve or send data.

### Web Scraping Tools

Software designed for web scraping, such as Scrapy for Python, makes HTTP requests to retrieve web pages for processing and data extraction.

### IoT Devices

Many Internet of Things devices, such as smart thermostats, security cameras, and wearables, communicate with servers via HTTP to send or receive data.

## resources

- Web browser: https://en.wikipedia.org/wiki/Web_browser
- Client (computing): https://en.wikipedia.org/wiki/Client_(computing)

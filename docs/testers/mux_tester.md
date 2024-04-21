# Concurrent GET Requests Program Documentation

This program demonstrates a simple approach to concurrency by sending GET requests to three specified endpoints concurrently.

## Installation and Setup

### Dependencies

- **libcurl:** This program requires libcurl to send HTTP requests. Make sure libcurl is installed and properly linked during compilation. You can install brew on 42 school computers.
  ```bash
  brew install libcurl
  ```

### Compiling the Program

- **On macOS:**

  - Install libcurl development files: `brew install libcurl4-openssl-dev`
  - Compile with:
    ```bash
    c++ -std=c++11 -I/usr/local/opt/curl/include -L/usr/local/opt/curl/lib test_mux.cpp -o test_mux -lcurl -pthread
    ```

- **On Linux:**
  - To find the installation path of curl, use:
    ```bash
    find / -name curl.h 2>/dev/null
    ```
  - Assuming libcurl is installed at `/home/lmangall/.brew/Cellar/curl/8.7.1/`, compile with:
    ```bash
    c++ -std=c++11 test_mux.cpp -o test_mux -I/home/lmangall/.brew/Cellar/curl/8.7.1/include -L/home/lmangall/.brew/Cellar/curl/8.7.1/lib -lcurl -pthread
    ```

### Creating a Test File

To test the server's multiplexing, we will have it handle a large file in one of the request. You can create a dummy file named `largefile` using the `dd` command:

```bash
dd if=/dev/zero of=var/www/development_site/largefile bs=50M count=1
```

### Example results

In this result we can see that the second requet, the one for the large file, blocked the third request

```bash
c4c6c1% ./test_mux
Sending request to http://localhost:8080
Sending request to http://localhost:8080/development_site/largefile
Sending request to http://localhost:8080
Connected. Response code: 200. Time taken: 0.68019 ms
Connected. Response code: 200. Time taken: 37101.2 ms
Connected. Response code: 200. Time taken: 37127.8 ms
```

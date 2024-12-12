### SSL Object (or SSL Structure)

The `SSL` object in OpenSSL represents a single SSL/TLS connection. It encapsulates all the data and state needed to manage an SSL/TLS session between a client and a server.

### Key Functions of the SSL Object

1. **Manage SSL/TLS Sessions**:

   - Handles the SSL/TLS handshake, which establishes the encryption parameters and authenticates the connection.
   - Manages the session state, including encryption keys, certificates, and negotiated parameters.

2. **Data Encryption and Decryption**:

   - Encrypts data sent over the connection, ensuring privacy and data integrity.
   - Decrypts incoming data, making it readable for the application.

3. **Communication**:
   - Provides functions for reading and writing encrypted data (`SSL_read`, `SSL_write`).
   - Handles underlying network communication securely.

### Common Operations with the SSL Object

- **Creation**: An `SSL` object is created using `SSL_new(SSL_CTX *ctx)`, where `ctx` is the SSL context that defines configuration and parameters for the connection.

  ```c
  SSL *ssl = SSL_new(SSL_CTX *ctx);
  ```

  Creates a new `SSL` object from the given context.

- **Association with Socket**: The `SSL` object is associated with a network socket using `SSL_set_fd(SSL *ssl, int fd)`, linking the SSL session to a specific socket file descriptor.

  ```c
  SSL_set_fd(SSL *ssl, int fd);
  ```

  Associates the `SSL` object with a network socket.

- **Handshake**: The SSL/TLS handshake is initiated using `SSL_accept` (for servers) or `SSL_connect` (for clients). This step negotiates the SSL/TLS parameters and establishes a secure connection.

  ```c
  int result = SSL_accept(SSL *ssl); // For servers
  int result = SSL_connect(SSL *ssl); // For clients
  ```

  Performs the SSL/TLS handshake.

- **Data Transfer**: Secure data transfer is performed using `SSL_read` to receive data and `SSL_write` to send data.

  ```c
  int bytes = SSL_read(SSL *ssl, void *buf, int num);
  int bytes = SSL_write(SSL *ssl, const void *buf, int num);
  ```

  Reads and writes encrypted data.

- **Shutdown**: The SSL connection is properly closed using `SSL_shutdown` to ensure all encrypted data is properly transmitted and received before the connection is terminated.

  ```c
  int result = SSL_shutdown(SSL *ssl);
  ```

  Properly shuts down the SSL connection.

- **Cleanup**: When the connection is no longer needed, `SSL_free` is called to free the `SSL` object and release associated resources.

  ```c
  SSL_free(SSL *ssl);
  ```

  Frees the `SSL` object and associated resources.

### Structure of the SSL Object

The `SSL` object in OpenSSL is a structure. Here is a partial definition to give you an idea of its components. For the complete structure, refer to the OpenSSL source code.

```c
struct ssl_st {
    // SSL session information
    SSL_CTX *ctx;
    // Underlying BIOs for network communication
    BIO *rbio; /* used by SSL_read */
    BIO *wbio; /* used by SSL_write */
    BIO *bbio; /* used during session-id reuse to concatenate messages */

    // SSL connection state
    int state;
    int rwstate;
    int in_handshake;
    int server; /* are we the server side? - mostly used by SSL_clear */
    int new_session; /* flag to indicate a new session */
    int quiet_shutdown;
    int shutdown; /* we have shut things down, 0x01 sent, 0x02 for received */
    int init_num; /* number of bytes in init_buf */
    int init_off; /* offset into init_buf */
    // and many more...
};
```

### Example

```cpp
SSL *ssl = SSL_new(ctx);          // Create a new SSL object
SSL_set_fd(ssl, socket_fd);       // Associate the SSL object with a socket
if (SSL_accept(ssl) <= 0) {       // Perform the SSL handshake
    // Handle error
}
SSL_write(ssl, data, data_len);   // Send encrypted data
SSL_read(ssl, buffer, buf_len);   // Receive encrypted data
SSL_shutdown(ssl);                // Properly close the SSL connection
SSL_free(ssl);                    // Free the SSL object and resources
```

## Further Information

For detailed information on the `SSL` object and related functions, refer to the following resources:

- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [OpenSSL `SSL` Object Manual](https://www.openssl.org/docs/man1.1.1/man3/SSL_new.html)
- [OpenSSL Source Code](https://github.com/openssl/openssl) (particularly the `ssl/ssl.h` file)

# Testing Build with Docker

This document provides instructions to build and run Docker images for testing the project with and without OpenSSL installed.

## Building Docker Images

### Build Image with OpenSSL Installed

From the root of the repository:

```sh
docker build -f tests/build/Dockerfile.with-openssl -t ubuntu-with-openssl .
```

### Build Image without OpenSSL Installed

From the root of the repository:

```sh
docker build -f tests/build/Dockerfile.without-openssl -t ubuntu-without-openssl .
```

## Running the Docker Images

### Run the Image with OpenSSL Installed

To run the Docker container with OpenSSL installed in interactive mode, use the following command:

```sh
docker run --rm -it ubuntu-with-openssl
```

### Run the Image without OpenSSL Installed

To run the Docker container without OpenSSL installed in interactive mode, use the following command:

```sh
docker run --rm -it ubuntu-without-openssl
```

## Test the build

`Make re` and check what happens.

# Docker Setup for C++ 

This project demonstrates setting up a simple C++ server in a Docker environment designed to handle connections on port 8080 from multiple IP addresses. The server setup is intended to validate the behavior of server sockets listening to all IP addresses and to a specific IP, both on the same port. Docker is used to create a controlled testing environment where such scenarios can be simulated.

## Files

- **Dockerfile**: Configuration file for building the Docker image.
- **overlapping.cpp**: Source code for the server.

## Dockerfile Explanation

The `Dockerfile` contains instructions for Docker on how to build the image for our server.

```Dockerfile
FROM ubuntu:latest            # Base image

WORKDIR /usr/src/app          # Set working directory in the container

COPY overlapping.cpp .        # Copy source code into the container

RUN apt-get update && apt-get install -y g++ curl   # Install g++ and curl

RUN g++ -o myserver overlapping.cpp  # Compile the source file

EXPOSE 8080                  # Expose port 8080 on the container

CMD ["./myserver"]            # Command to run the server when the container starts
```

### Build Steps

1. **FROM**: Sets the base image for subsequent instructions. In this case, `ubuntu:latest` is used as the base.
2. **WORKDIR**: Sets the working directory inside the container. All subsequent commands will be run from this directory.
3. **COPY**: Copies the server's source file `overlapping.cpp` into the container.
4. **RUN**: Executes commands in a new layer on top of the current image. Here it's used to update package lists, install necessary packages (`g++`, `curl`), and compile the C++ program.
5. **EXPOSE**: Informs Docker that the container listens on the specified network ports at runtime.
6. **CMD**: Provides the default command to be executed when the container starts.

## STEPS

## 1. Building the Docker Image

Build the Docker image from the Dockerfile:

```bash
docker build -t my_server_img .
```

- **`docker build`**: Command to build the Docker image.
- **`-t my_server_image`**: Tags the image with a name so it can be easily referenced.
- **`.`**: Specifies the directory of the Dockerfile (current directory in this case).

## 2 Run the Docker Container in Interactive Mode

To actively monitor and interact with the server output, start the container in interactive mode and manually initiate the server: the Docker container using the image you've built:

```bash
docker run --rm -it --name my_server_container -p 8080:8080 my_server_img
```

- **`docker run`**: Command to run the Docker container.
- **`--rm`**: Automatically remove the container when it exits.
- **`-d`**: Run the container in detached mode (in the background).
- **`--name my_-server_container`**: Names the container for easy reference.
- **`-p 8080:8080`**: Publishes the container's port 8080 to the host.
- **`my_server_image`**: Specifies which image to use for the container.

If you want to see the output on the terminal just don't use the `-d` flag.

## 3. Start the Server

Add manually 127.0.0.2 to the list of the hosts:
`echo "127.0.0.2 localhost2" | sudo tee -a /etc/hosts`

We can't do this directly from the Dockerfile


`./myserver`

## 4. Testing the Server

`docker exec -it my_server_container bash`


Once the container is running, you can test the server using:

```bash
curl http://localhost:8080
curl http://127.0.0.2:8080
```

This sends an HTTP request to your server running inside the Docker container.

## 5. Cleanup

`docker stop my_server_container`

# Nginx Server Docker Container

The Nginx source code has been downloaded from the [Ngnix website](https://nginx.org/en/download.html).

The Dockerfie in this directory is building an Ubunut environemnt and building Ngnix from source. The advantage is to have the source code, directly in the directory and be able to test Ngnix in a Linux environemnte. Ngnix is the webserver we take as reference in our webserv project.

## Prerequisites

- Docker installed on your machine. For installation instructions, visit the [Docker website](https://docs.docker.com/get-docker/).

## Building the Docker Image

To build the Docker image:

1. Open a terminal and navigate to the directory containing the Dockerfile.
2. Execute the following command:

   ```
   docker build -t nginx-test .
   ```

This command builds a Docker image named `nginx-test` based on the instructions in the Dockerfile.

## Running the Docker Container

Run the Nginx server inside the Docker container on a custom port (e.g., 8888) with the following command:

```
docker run -d -p 8888:80 nginx-test
```

This command maps port 8888 on your host machine to port 80 inside the container, where Nginx is configured to listen by default. We choose to use port 8888 on the host to avoid conflicts with other services, including our development web server, which might already be using another port. See the section `Understanding port mapping` for a more detailed explanation of what is going on.

## Verifying the Installation

To verify that the Nginx server is running:

1. Open a web browser.
2. Navigate to `http://localhost:8888`.

If the setup is correct, the default Nginx welcome page will be displayed, indicating that the server is running and accessible.

## Understanding port mapping

When you run a Docker container and specify port mappings using `-p <host_port>:<container_port>`, Docker creates a bridge between a specified port on your host machine and a port inside the Docker container. This mechanism is crucial for containerized applications that need to be accessible from outside the Docker environment, such as web servers.

- **Host Port (`<host_port>`):** This is the port on your host machine. When you access this port, Docker forwards the traffic to a corresponding port inside the container. This allows you to interact with the Dockerized application using your host machine's IP address or localhost.

- **Container Port (`<container_port>`):** This port is within the Docker container, where the application (in this case, Nginx) listens for incoming connections or requests. Applications inside containers are configured to listen on specific ports, just as they would on a physical or virtual server.

### The Command Explained

```bash
docker run -d -p 8888:80 nginx-test
```

This command tells Docker to run the `nginx-test` container in detached mode (`-d`), creating a pathway for traffic between the host's port 8888 and the container's port 80. Here's why each part matters:

- **`-d`**: This flag runs the container in the background (detached mode), allowing you to continue using the terminal session.

- **`-p 8888:80`**: This port mapping is critical for making Nginx accessible:

  - **`8888` (Host Port):** We've chosen this non-standard web port to avoid conflicts with any existing services on the host, such as another web server or development tools that might be using more common ports like 80 or 8080. Using port 8888 minimizes the chance of port collisions, which can cause errors or prevent services from starting.

  - **`80` (Container Port):** By default, Nginx is configured to listen on port 80 for HTTP requests. When we map host port 8888 to this port, any traffic sent to `http://localhost:8888` is forwarded internally by Docker to port 80 of the running `nginx-test` container. This setup ensures that Nginx can handle requests without requiring any changes to its default configuration.

### Practical Implication

By mapping port 8888 on your host to port 80 inside the Nginx container, you're effectively exposing Nginx to users and systems on your network (or the internet, if your host is publicly accessible) through a non-standard port. This approach allows you to run multiple web servers or services simultaneously without port conflicts, facilitating a smoother development and testing process.

# Web Client in Go

This project contains a Dockerized Go client, `webclient.go`, designed to send HTTP requests to a specified web server. The client is containerized to ensure consistency across development environments and ease of deployment.

## Prerequisites

- Docker must be installed on your system. Visit [Docker's official website](https://www.docker.com/get-started) for installation instructions.
- Ensure you have basic familiarity with Docker commands and concepts.
- If you intend to communicate with a web server, ensure it's accessible from your machine.

## Building the Docker Image

To build the Docker image for the Go client, navigate to the directory containing the `Dockerfile` and `webclient.go`, then run the following command:

```sh
docker build -t webclient-image .
```

This command builds the Docker image with the tag `webclient-image` using the Dockerfile in the current directory.

## Running the Docker Container

After successfully building the image, you can run the Docker container using the following command:

```sh
docker run webclient-image
```

This command starts a container instance of `webclient-image`. If your client needs to communicate with a specific web server, make sure the Go client is configured with the correct server address and port.

## Interacting with a Web Server

The `webclient.go` is designed to send HTTP requests. Ensure the target web server's address and port are correctly specified in the Go code. If modifications are required, update `webclient.go`, rebuild the Docker image, and run the container again.

## Debugging and Logs

To view logs from the running container, use the Docker logs command:

```sh
docker logs <container_id>
```

The `<container_id>` can be found by listing all running containers with `docker ps`.

## Additional Commands

- **Stopping a Container**: To stop a running container, use `docker stop <container_id>`.
- **Removing a Container**: To remove a stopped container, use `docker rm <container_id>`.
- **Listing Images**: To see all available Docker images, use `docker images`.
- **Removing an Image**: To remove an unused Docker image, use `docker rmi <image_name>`.

## Contributing

If you wish to contribute to the project or modify the Go client, please follow the standard Git workflow:

1. Fork the repository.
2. Create a new branch for your feature or fix.
3. Commit your changes with descriptive commit messages.
4. Push your branch and submit a pull request to the main project.

For major changes, please open an issue first to discuss what you would like to change.

---

This README provides a comprehensive guide for your team to build, run, and interact with the `webclient` Docker container. Make sure to keep the documentation updated with any changes to the project setup or functionality.

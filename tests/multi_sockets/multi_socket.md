Sure, here's the revised version with minor adjustments for clarity:

### Docker Test for Multi Server-Sockets

The dockerization is necessary because on the school computer, we don't have sudo rights, and we don't have the possibility to add another localhost address (like 127.0.0.2) to test the capability of the server to bind to multiple IPs. We can test with multiple ports, though.

The Dockerfile is located in `/tests/multi_sockets`, but we need to build the image from the root of the project. Then, we will start a container in interactive mode and run a bash script that will execute the tests.

In the `multi.conf` file, we are listening on:

- 0.0.0.0:8081
- 127.0.0.1:8080
- 127.0.0.1:8082

We send requests to:

- `127.0.0.1:8080`: we get a "404", which means success
- `0.0.0.0:8081`: "404", same
- `127.0.0.1:8082`: "404", same
- `127.0.0.2:8080`: "000", this one fails because we are not listening on 127.0.0.2:8080
- `127.0.0.3:8080`: "000"
- `127.0.0.1:8081`: "404"

### Docker IPv6 Setup Guide

#### 0. Enable IPv6 in Docker

To enable IPv6 support in Docker, follow these steps:

1. **Edit Docker daemon configuration**:
   Edit the `/etc/docker/daemon.json` file to include the necessary IPv6 parameters:

   ```json
   {
     "ipv6": true,
     "fixed-cidr-v6": "2001:db8:1::/64",
     "experimental": true,
     "ip6tables": true
   }
   ```

   Step 1 on Mac needs to be done in the Desktop app > Settings > Engine

2. **Restart Docker**:
   Save the configuration file and restart the Docker daemon for the changes to take effect:

   ```sh
   sudo systemctl restart docker
   ```

3. **Create an IPv6 network**:
   Create a Docker network that uses IPv6:

   ```sh
   docker network create --ipv6 --subnet 2001:db8:1::/64 ip6net
   ```

#### 1. Build the Docker Image

From the root of `webserv`:

```sh
docker build -t multi_sockets -f tests/multi_sockets/Dockerfile .
```

#### 2. Run the Docker Container

```sh
docker run -it --rm multi_sockets /bin/bash
```

#### 3. Start the Test Script

Once inside the container:

```sh
./test.sh
```

By following these steps, you will build the Docker image, run the container interactively, and execute the test script to verify the functionality of your web server.

#Docker

### 1. List Docker Containers

To see all currently running Docker containers, use:

```bash
docker ps
```

To see both running and stopped containers, use:

```bash
docker ps -a
```

### 2. Inspect Container Networking

Each Docker container can be inspected to reveal its network settings, including IP address, network mode, and linked containers. To inspect a container, use:

```bash
docker inspect [container_name_or_id]
```

This command will output a JSON array containing a lot of information about the container. To specifically see networking details, you can filter the output with tools like `grep`, `awk`, or `jq` (for JSON parsing). For example:

```bash
docker inspect [container_name_or_id] | grep IPAddress
```

### 3. List Docker Networks

Docker manages its own networks. To see a list of all Docker networks:

```bash
docker network ls
```

To inspect a specific network and see which containers are connected to it:

```bash
docker network inspect [network_name]
```

### 4. Connect to a Container

If you need to connect to a container—for example, to access a shell inside the container—use:

```bash
docker exec -it [container_name_or_id] bash
```

This assumes that the container has `bash` installed. If not, you might use `sh` or another shell available in the container.

### 5. Accessing Container Services

If your container is running a service on a certain port and you've mapped that port to a port on the host (using `-p host_port:container_port` when you started the container), you can access that service using:

```bash
curl http://localhost:[host_port]
```

or by entering `http://localhost:[host_port]` in a web browser on the host machine.

### 6. Check Network Interfaces

Docker typically uses a virtual bridge (usually named `docker0`) to manage container networking. You can see this bridge and other network interfaces using:

```bash
ifconfig
```

or

```bash
ip addr show
```

### 7. Testing Network Connectivity

You can test network connectivity to and from a container using tools like `ping` or `curl` from within the container (using `docker exec`) or from the host to the container's exposed ports.

### Example Commands

Here’s how you might use these commands in practice:

- **List all networks and inspect a network:**
  ```bash
  docker network ls
  docker network inspect bridge
  ```

- **Inspect a container for its IP address:**
  ```bash
  docker inspect my_container --format '{{ .NetworkSettings.IPAddress }}'
  ```

These tools and commands will give you a comprehensive view of how Docker is configured and running on your Linux system, allowing you to manage and troubleshoot containers effectively.
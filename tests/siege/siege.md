# Test Webserv with Siege in a Docker Environment

## 1. Build the Docker Image

Use the Dockerfile in this folder. 

From the **root** directory build the Docker image:

```bash
docker build -t ubuntu-siege -f tests/siege/Dockerfile .
```

## 2. Start the first interactive session 

1. Run Siege interactively within the container, use:

```bash
docker run --name ubuntu-siege-session --rm -it -v "$(pwd):/app" ubuntu-siege /bin/bash

```

## 3. Compile the webserver 

    `make re`

### 4. Start a second interactive section

```bash
docker exec -it ubuntu-siege-session /bin/bash
```

### 5. Stress test the webserv


Inside the container, you can execute commands like:

```bash
siege -c10 -d5 -t1M http://localhost:80
```

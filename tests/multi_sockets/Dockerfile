# Use a base image with C++ and necessary tools installed
FROM ubuntu:20.04

# Install necessary packages
RUN apt-get update && \
    apt-get install -y build-essential curl vim iproute2 net-tools

# Set the working directory
WORKDIR /app

# Copy the entire project into the containerenet-tools
COPY . .  

# Compile the project in the root directory where the Makefile is located
RUN make re -C /app
# Compile the project
RUN make

# Change to the directory containing the multi.conf file
WORKDIR /app/tests/multi_sockets

# Make the test script executable
RUN chmod +x testIPv4.sh
RUN chmod +x testIPv6.sh
RUN chmod +x testIPv4+IPv6.sh 
RUN chmod +x test_mixed.sh

# Enable IPv6 in the container
RUN sysctl -w net.ipv6.conf.all.disable_ipv6=0 && \
    sysctl -w net.ipv6.conf.default.disable_ipv6=0 && \
    sysctl -w net.ipv6.conf.lo.disable_ipv6=0

# Just start the shell
CMD ["/bin/bash"]

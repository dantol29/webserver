# Use a base image with C++ and necessary tools installed
FROM ubuntu:20.04

# Install necessary packages
RUN apt-get update && \
    apt-get install -y build-essential curl vim

# Set the working directory
WORKDIR /app

# Copy the entire project into the container
COPY . .  


# List the contents of the /app directory to verify the presence of the Makefile
RUN echo "Contents of /app:" && ls -al /app

# List the contents of the tests directory to verify the structure
RUN echo "Contents of /app/tests:" && ls -al /app/tests

# List the contents of the multi_sockets directory to verify the structure
RUN echo "Contents of /app/tests/multi_sockets:" && ls -al /app/tests/multi_sockets

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

# Just start the shell
CMD ["/bin/bash"]

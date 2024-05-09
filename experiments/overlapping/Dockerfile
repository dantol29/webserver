# Use the latest Ubuntu base image
FROM ubuntu:latest

# Set the working directory in the container
WORKDIR /usr/src/app

# Copy the server's source file into the container
COPY overlapping.cpp .

# Update package lists, install necessary packages and compile the C++ program
RUN apt-get update && \
    apt-get install -y g++ curl && \
    g++ -o myserver overlapping.cpp

# Inform Docker that the container listens on port 8080
EXPOSE 8080

# Command to run the server when the container starts
CMD ["./myserver"]

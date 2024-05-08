FROM ubuntu:latest
# Update packages and install dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        curl \
        build-essential \
    && rm -rf /var/lib/apt/lists/*

COPY . /src
WORKDIR /src


# Compile the program
RUN g++ -o my_program overlapping.cpp


# Start the compiled program
CMD ["./my_program"]
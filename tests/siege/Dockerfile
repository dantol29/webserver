FROM ubuntu:latest

# Update packages and install necessary tools
RUN apt-get update && \
	apt-get install -y \
	build-essential \ 
	# Includes g++ and make
	git \
	curl \
	vim \ 
	siege \
	# Optional, for editing inside the container
	&& rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /app

# Default command or entrypoint
CMD ["bash"]

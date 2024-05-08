# Use a base image
FROM ubuntu:latest

# Install dependencies
RUN apt-get update && \
    apt-get install -y build-essential libpcre3 libpcre3-dev zlib1g zlib1g-dev libssl-dev

# Copy the Nginx source code from your repository to the image
COPY nginx-1.24.0 /usr/src/nginx-1.24.0

# Compile Nginx
WORKDIR /usr/src/nginx-1.24.0
RUN ./configure --without-http_gzip_module && \
    make && \
    make install

# Set the PATH
ENV PATH="/usr/local/nginx/sbin:$PATH"

# Forward request logs to Docker log collector
RUN ln -sf /dev/stdout /usr/local/nginx/logs/access.log \
    && ln -sf /dev/stderr /usr/local/nginx/logs/error.log

# Expose port 80
EXPOSE 80

# Start Nginx in the foreground
CMD ["nginx", "-g", "daemon off;"]

# Use a base image
FROM ubuntu:latest

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Install dependencies
RUN apt-get update && \
    apt-get install -y build-essential libpcre3 libpcre3-dev zlib1g zlib1g-dev libssl-dev php-fpm php-mysql tzdata

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

# Copy Nginx config file
COPY nginx.conf /usr/local/nginx/conf/nginx.conf

# Start Nginx and PHP-FPM
CMD service php7.4-fpm start && nginx -g 'daemon off;'

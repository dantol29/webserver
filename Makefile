# Compiler and Flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I. -Iinclude -Isrc -Isrc/events -Isrc/ssl -g
DEPFLAGS = -MMD -MP
LDFLAGS =

UNAME_S := $(shell uname -s)
# Additional Flags for macOS
ifeq ($(UNAME_S), Darwin)
    CXXFLAGS += -fsanitize=address
    LDFLAGS += -fsanitize=address
endif

# Check if pkg-config is available and OpenSSL is installed
PKGCONFIG := $(shell which pkg-config)
USE_LOCAL_OPENSSL := 0
ifneq ($(PKGCONFIG),)
    PKGCONFIG_OPENSSL := $(shell pkg-config --exists openssl && echo 1 || echo 0)
    ifeq ($(PKGCONFIG_OPENSSL), 0)
        USE_LOCAL_OPENSSL := 1
    else
        OPENSSL_INCLUDE := $(shell pkg-config --cflags-only-I openssl | sed 's/-I//')
        OPENSSL_LIB := $(shell pkg-config --libs-only-L openssl | sed 's/-L//')
        CXXFLAGS += -I$(OPENSSL_INCLUDE)
        LDFLAGS += -L$(OPENSSL_LIB) -lssl -lcrypto
    endif
else
    USE_LOCAL_OPENSSL := 1
endif

# Paths for local OpenSSL installation
LOCAL_OPENSSL_DIR := $(CURDIR)/local/openssl
LOCAL_INCLUDE := $(LOCAL_OPENSSL_DIR)/include
LOCAL_LIB := $(LOCAL_OPENSSL_DIR)/lib

# Source and Object Files
SRCS = src/main.cpp \
	src/Parser.cpp \
	src/Config.cpp \
	src/Connection.cpp \
	src/Server.cpp \
	src/server_utils.cpp \
	src/HTTPRequest.cpp \
	src/HTTPResponse.cpp \
	src/Router.cpp \
	src/AResponseHandler.cpp \
	src/StaticContentHandler.cpp \
	src/CGIHandler.cpp \
	src/MetaVariables.cpp \
	src/UploadHandler.cpp \
	src/Debug.cpp \
	src/utils.cpp \
	src/ServerBlock.cpp \
	src/ServerSocket.cpp \
	src/Listen.cpp \
	src/events/EventManager.cpp \
	src/events/ServerEventListener.cpp \
	src/ssl/SSLManager.cpp \
	src/ssl/SSLContext.cpp 
OBJDIR = obj
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

# Main Target
TARGET = webserv

# Build Rules
all: check_openssl $(TARGET)

# Rule to check and install OpenSSL if needed
check_openssl:
ifeq ($(INSTALL_OPENSSL), 1)
	@echo "OpenSSL not found. Installing locally..."
	@$(MAKE) install_openssl
else
	@echo "Using system OpenSSL."
endif

# Rule to download, build, and install OpenSSL locally
install_openssl:
	@mkdir -p openssl
	@cd openssl && \
	wget https://www.openssl.org/source/openssl-1.1.1.tar.gz && \
	tar -xzf openssl-1.1.1.tar.gz && \
	cd openssl-1.1.1 && \
	./config --prefix=$(LOCAL_OPENSSL_DIR) --openssldir=$(LOCAL_OPENSSL_DIR) no-shared && \
	make && \
	make install && \
	cd ../.. && \
	rm -rf openssl

# Ensure the necessary directories exist before compiling anything
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Linking the main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)  $(LDFLAGS)

# Cleaning up the build
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re check_openssl install_openssl
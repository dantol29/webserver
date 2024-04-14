# Compiler and Flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I. -Iinclude -Isrc
DEPFLAGS = -MMD -MP

# Source and Object Files
SRCS = src/main.cpp \
	src/Connection.cpp \
	src/CGI_handler.cpp \
	src/CGIHandler.cpp \
	src/Environment.cpp \
	src/utils.cpp \
	src/HTTPRequest.cpp \
	src/HTTPResponse.cpp \
	src/Router.cpp \
	src/server_utils.cpp \
	src/Server.cpp \
	src/StaticContentHandler.cpp \
	src/ConfigFile.cpp \
	src/get_next_line.cpp
OBJDIR = obj
OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

# Main Target
TARGET = webserv

# Build Rules
all: $(TARGET)

# Ensure the necessary directories exist before compiling anything
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Linking the main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Cleaning up the build
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re

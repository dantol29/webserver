# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I. -Iinclude
DEPFLAGS = -MMD -MP

# Source and Object Files
SRCS = main.cpp
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

# Main Target
TARGET = webserv

# CGI Target
CGI_TARGET = cgi-bin/hello.cgi
CGI_SRC = cgi-bin/hello.cpp

# Ensure the OBJDIR exists before compiling anything
$(shell mkdir -p $(OBJDIR))

# Rule for compiling source files into object files
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Build Rules
all: $(TARGET) $(CGI_TARGET)

# Linking the main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compiling the CGI script
$(CGI_TARGET): $(CGI_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Cleaning up the build
clean:
	rm -rf $(OBJDIR)
	rm -f $(TARGET) $(CGI_TARGET)

fclean: clean
	rm -f $(TARGET) $(CGI_TARGET)

re: fclean all

.PHONY: all clean fclean re

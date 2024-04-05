# Compiler and Flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I. -Iinclude
DEPFLAGS = -MMD -MP

# Source and Object Files
SRCS = main.cpp \
	Server.cpp \
	RequestHandler.cpp \
	CGIHandler.cpp \
	Environment.cpp \
	Utils.cpp
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

# Main Target
TARGET = webserv

# Ensure the OBJDIR exists before compiling anything
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rule for compiling source files into object files
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Build Rules
all: $(TARGET)

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

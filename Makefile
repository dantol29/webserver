# Compiler and Flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I.
DEPFLAGS = -MMD -MP

# Target executable name
TARGET = webserv

# Source and Object Files
SRCS = main.cpp HTTPRequest.cpp utils.cpp

# Create an obj directory if it doesn't exist
OBJDIR = obj

# Generate object files from source files in the obj directory
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
# Build Rules

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)
# Compile source files to object files in the obj directory
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR)
	rm -f $(OBJS) $(TARGET) 

fclean: clean
	rm -f $(TARGET) 

re: fclean all

.PHONY: all  clean fclean re

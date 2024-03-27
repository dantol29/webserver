CXX=g++
CXXFLAGS=-Wall -Wextra -Werror -std=c++98
LDFLAGS=-pthread
TARGET=server

all: $(TARGET)

$(TARGET): server.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) server.cpp $(LDFLAGS)

clean:
	rm -f $(TARGET)

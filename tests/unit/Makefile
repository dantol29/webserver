# Simple Makefile for building Listen tests

# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -I../../ -I../../include -I../../src 

# Build and link settings
SRC_DIR = ../../src
TEST_DIR = .
BUILD_DIR = build

# Source files
SOURCES = $(SRC_DIR)/ServerBlock.cpp $(SRC_DIR)/utils.cpp $(TEST_DIR)/listen.cpp $(SRC_DIR)/Debug.cpp $(SRC_DIR)/Listen.cpp
OBJECTS = $(BUILD_DIR)/ServerBlock.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/listen.o $(SRC_DIR)/Debug.o $(SRC_DIR)/Listen.o
TARGET = $(BUILD_DIR)/test_listen

all: $(TARGET)

$(SRC_DIR)/Listen.o: $(SRC_DIR)/Listen.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ServerBlock.o: $(SRC_DIR)/ServerBlock.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/listen.o: $(TEST_DIR)/listen.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SRC_DIR)/Debug.o: $(SRC_DIR)/Debug.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)



run: all
	@./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all run clean

# Ensure directories exist
$(shell mkdir -p $(BUILD_DIR))

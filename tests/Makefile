# Makefile for mux.cpp

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -pthread

# Targets
TARGET = mux
SOURCES = mux.cpp

# largefile irectory setup
OUTPUT_DIR = ../var/www/development_site
LARGE_FILE = $(OUTPUT_DIR)/largefile

all: $(TARGET) largefile

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

largefile: $(OUTPUT_DIR)
	dd if=/dev/zero of=$(LARGE_FILE) bs=50M count=1

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -f $(TARGET) $(LARGE_FILE)

.PHONY: all clean largefile

# Compiler
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
NET_DIR = $(SRC_DIR)/net
UTILS_DIR = $(SRC_DIR)/utils

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(CORE_DIR)/ftp_server.cpp \
          $(CORE_DIR)/ftp_client.cpp \
          $(CORE_DIR)/ftp_protocol.cpp \
          $(NET_DIR)/socket.cpp \
          $(UTILS_DIR)/utility.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable
TARGET = cpftp

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build complete: $(TARGET)"

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete"

# Rebuild everything
rebuild: clean all

# Phony targets
.PHONY: all clean rebuild

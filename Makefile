# Iris Build System - Makefile
# Build the Iris build tool itself

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter -O2
LDFLAGS := -pthread

# Debug build
ifdef DEBUG
    CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g -O0 -DDEBUG
endif

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Source files
SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# Target
TARGET := $(BIN_DIR)/iris

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	@echo "  LINK    $@"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "  Build complete: $@"
	@echo ""

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "  CXX     $<"
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPENDS)

# Clean
clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install
install: $(TARGET)
	@echo "  INSTALL $(TARGET) -> /usr/local/bin/iris"
	@install -m 755 $(TARGET) /usr/local/bin/iris

# Uninstall
uninstall:
	@echo "  UNINSTALL /usr/local/bin/iris"
	@rm -f /usr/local/bin/iris

# Run
run: $(TARGET)
	@./$(TARGET) --help

# Test
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET) --version
	@echo "Tests passed!"

# Format
format:
	@echo "Formatting source files..."
	@find $(SRC_DIR) -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Debug build
debug:
	@$(MAKE) DEBUG=1

# Release build
release:
	@$(MAKE) CXXFLAGS="-std=c++17 -O3 -DNDEBUG -march=native -flto"

# Help
help:
	@echo "Iris Build System - Build Targets"
	@echo ""
	@echo "  make          Build iris (default)"
	@echo "  make debug    Build with debug symbols"
	@echo "  make release  Build optimized release"
	@echo "  make clean    Remove build artifacts"
	@echo "  make install  Install to /usr/local/bin"
	@echo "  make test     Run tests"
	@echo "  make format   Format source code"
	@echo "  make help     Show this help"
	@echo ""

.PHONY: all clean install uninstall run test format debug release help

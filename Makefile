# the iris build system makefile fallback
# built, and remains a backup for iris build system if self hosting screws up (hopefully not)

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter -O2
LDFLAGS := -pthread

# debug build
ifdef DEBUG
    CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g -O0 -DDEBUG
endif

# directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# source files
SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# target
TARGET := $(BIN_DIR)/iris

# default target
all: $(TARGET)

# link
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	@echo "  LINK    $@"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo ""
	@echo "  Build complete: $@"
	@echo ""

# compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "  CXX     $<"
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# include dependencies
-include $(DEPENDS)

# clean
clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

# install
install: $(TARGET)
	@echo "  INSTALL $(TARGET) -> /usr/local/bin/iris"
	@install -m 755 $(TARGET) /usr/local/bin/iris

# uninstall
uninstall:
	@echo "  UNINSTALL /usr/local/bin/iris"
	@rm -f /usr/local/bin/iris

# run (dude i document so much in my comments, but it makes debugging easier)
run: $(TARGET)
	@./$(TARGET) --help

# test
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET) --version
	@echo "Tests passed!"

# format
format:
	@echo "Formatting source files..."
	@find $(SRC_DIR) -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# debug build
debug:
	@$(MAKE) DEBUG=1

# release build
release:
	@$(MAKE) CXXFLAGS="-std=c++17 -O3 -DNDEBUG -march=native -flto"

# help
help:
	@echo "Iris Build System, Build Targets"
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

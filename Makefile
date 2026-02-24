CXX := g++
CC := gcc
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Wno-unused-parameter
LDFLAGS := -pthread

ifdef DEBUG
    CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG -Wno-unused-parameter
endif

SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

CXX_SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
ASM_SOURCES := $(shell find $(SRC_DIR) -name '*.S' -o -name '*.s')

CXX_OBJECTS := $(CXX_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
ASM_OBJECTS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(filter %.S,$(ASM_SOURCES)))
ASM_OBJECTS += $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(filter %.s,$(ASM_SOURCES)))

OBJECTS := $(CXX_OBJECTS) $(ASM_OBJECTS)
DEPENDS := $(CXX_OBJECTS:.o=.d)

TARGET := $(BIN_DIR)/iris

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	@echo "  LINK    $@"
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "  CXX     $<"
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	@echo "  ASM     $<"
	@$(CC) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	@echo "  ASM     $<"
	@$(CC) -c $< -o $@

-include $(DEPENDS)

clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

install: $(TARGET)
	@echo "  INSTALL $(TARGET) -> /usr/local/bin/iris"
	@install -m 755 $(TARGET) /usr/local/bin/iris

debug:
	@$(MAKE) DEBUG=1

.PHONY: all clean install debug

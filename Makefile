CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./src/cli

SRC = src/cli src
BUILDIR = build
BIN = bin

TARGET = $(BIN)/iris

SRCS = $(foreach dir,$(SRC),$(wildcard $(dir)/*.cpp))
OBJS = $(patsubst %.cpp,$(BUILDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)


$(BUILDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDIR) $(TARGET)

.PHONY: all clean


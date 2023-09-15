CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2
SRC_DIR = src
OUT_DIR = out

# List of source files without extensions
SOURCES = producer_consumer mrsw

# Build targets for each source file
TARGETS = $(addprefix $(OUT_DIR)/, $(SOURCES))

# Default target is to build all targets
all: $(TARGETS)

# Build target for each source file
$(OUT_DIR)/producer_consumer: $(SRC_DIR)/producer_consumer.cpp
	mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(OUT_DIR)/mrsw: $(SRC_DIR)/mrsw.cpp
	mkdir -p $(OUT_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean

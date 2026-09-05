CXX ?= c++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude
BUILD := build
CORE := src/sim.cpp
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
LIB := $(BUILD)/libcreature.dylib
SHARED := -dynamiclib
else
LIB := $(BUILD)/libcreature.so
SHARED := -shared
endif
.PHONY: all core test viewer clean
all: core viewer
$(BUILD):
	mkdir -p $(BUILD)
core: $(LIB) $(BUILD)/sim_tests $(BUILD)/benchmark
$(LIB): $(CORE) src/api.cpp include/creature/sim.hpp include/creature/api.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC $(SHARED) $(CORE) src/api.cpp -o $@
$(BUILD)/sim_tests: $(CORE) tests/sim_tests.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/sim_tests.cpp -o $@
$(BUILD)/benchmark: $(CORE) tests/benchmark.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/benchmark.cpp -o $@
viewer: $(BUILD)/creature_lab
$(BUILD)/creature_lab: $(CORE) client/main.cpp client/font.hpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) client/main.cpp $$(sdl2-config --cflags --libs) -o $@
test: core
	$(BUILD)/sim_tests
	python3 python/smoke.py
clean:
	rm -rf $(BUILD)

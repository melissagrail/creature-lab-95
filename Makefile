CXX ?= c++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude
BUILD := build
CORE := src/sim.cpp src/content.cpp
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
$(BUILD)/creature_lab: $(CORE) client/main.cpp client/font.hpp client/tinikami.hpp client/garden.hpp client/spirit_rects.hpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) client/main.cpp $$(sdl2-config --cflags --libs) -o $@
test: core $(BUILD)/environment_tests
	python3 scripts/compile_content.py --check
	$(BUILD)/sim_tests
	$(BUILD)/environment_tests
	python3 python/smoke.py
clean:
	rm -rf $(BUILD)

.PHONY: content balance
content:
	python3 scripts/compile_content.py
$(BUILD)/tournament: $(CORE) tests/tournament.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/tournament.cpp -o $@
balance: $(BUILD)/tournament
	$(BUILD)/tournament 72 reports/matches.csv
	python3 scripts/analyze_balance.py reports/matches.csv reports/balance
$(BUILD)/counterplay: $(CORE) tests/counterplay.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/counterplay.cpp -o $@

$(BUILD)/environment_tests: $(CORE) tests/environment_tests.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/environment_tests.cpp -o $@

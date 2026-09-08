CXX ?= c++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic
CPPFLAGS += -Iinclude
BUILD := build
CORE := src/sim.cpp src/content.cpp
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
LIB := $(BUILD)/libcreature.dylib
BRAINLIB := $(BUILD)/libtinibrain.dylib
SHARED := -dynamiclib
else
LIB := $(BUILD)/libcreature.so
BRAINLIB := $(BUILD)/libtinibrain.so
SHARED := -shared
endif
.PHONY: all core test viewer clean
all: core viewer brain
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
$(BUILD)/creature_lab: $(CORE) client/main.cpp agents/brain.cpp include/creature/brain.hpp include/creature/brain_api.h client/font.hpp client/tinikami.hpp client/garden.hpp client/spirit_rects.hpp client/journey.hpp client/journey_audio.hpp client/design_notebook.hpp include/creature/campaign.hpp src/campaign.cpp src/campaign_content.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) src/campaign.cpp src/campaign_content.cpp agents/brain.cpp client/main.cpp $$(sdl2-config --cflags --libs) -o $@
test: core $(BUILD)/environment_tests $(BUILD)/brain_tests $(BUILD)/campaign_tests
	python3 scripts/compile_content.py --check
	python3 scripts/compile_campaign.py --check
	$(BUILD)/campaign_tests
	$(BUILD)/sim_tests
	$(BUILD)/environment_tests
	$(BUILD)/brain_tests models/apprentice.tbrain
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

.PHONY: brain
brain: $(BRAINLIB)
$(BRAINLIB): agents/brain.cpp src/content.cpp include/creature/brain.hpp include/creature/brain_api.h include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC $(SHARED) agents/brain.cpp src/content.cpp -o $@

$(BUILD)/brain_tests: $(CORE) agents/brain.cpp tests/brain_tests.cpp include/creature/brain.hpp include/creature/brain_api.h include/creature/replay.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) agents/brain.cpp tests/brain_tests.cpp -o $@

$(BUILD)/brain_eval: $(CORE) agents/brain.cpp tests/brain_eval.cpp include/creature/brain.hpp include/creature/brain_api.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) agents/brain.cpp tests/brain_eval.cpp -o $@

$(BUILD)/brain_probe: $(CORE) agents/brain.cpp tests/brain_probe.cpp include/creature/brain.hpp include/creature/brain_api.h | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) agents/brain.cpp tests/brain_probe.cpp -o $@

$(BUILD)/campaign_tests: $(CORE) src/campaign.cpp src/campaign_content.cpp tests/campaign_tests.cpp include/creature/campaign.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) src/campaign.cpp src/campaign_content.cpp tests/campaign_tests.cpp -o $@

$(BUILD)/campaign_playthrough: $(CORE) src/campaign.cpp src/campaign_content.cpp agents/brain.cpp tests/campaign_playthrough.cpp include/creature/campaign.hpp include/creature/brain.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) src/campaign.cpp src/campaign_content.cpp agents/brain.cpp tests/campaign_playthrough.cpp -o $@

$(BUILD)/footwork_benchmark: $(CORE) tests/footwork_benchmark.cpp include/creature/sim.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) tests/footwork_benchmark.cpp -o $@

$(BUILD)/apprenticeship_playthrough: $(CORE) src/campaign.cpp src/campaign_content.cpp agents/brain.cpp tests/apprenticeship_playthrough.cpp include/creature/campaign.hpp include/creature/brain.hpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORE) src/campaign.cpp src/campaign_content.cpp agents/brain.cpp tests/apprenticeship_playthrough.cpp -o $@

$(BUILD)/journey_audio_tests: tests/journey_audio_tests.cpp client/journey_audio.hpp | $(BUILD)
	$(CXX) $(CXXFLAGS) tests/journey_audio_tests.cpp $$(sdl2-config --cflags --libs) -o $@

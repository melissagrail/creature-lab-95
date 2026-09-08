#include "creature/brain.hpp"
#include "creature/replay.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
using namespace creature;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        ++checks;                                                                                  \
        if (!(x)) {                                                                                \
            std::cerr << "brain failure " << __LINE__ << ": " #x "\n";                             \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(int argc, char **argv) {
    int checks = 0;
    Brain brain;
    std::string error;
    CHECK(brain.load(argc > 1 ? argv[1] : "models/apprentice.tbrain", error));
    uint32_t identity = brain.checksum();
    CHECK(!brain.load("no-such-brain.tbrain", error));
    CHECK(brain.ready() && brain.checksum() == identity);
    CHECK(personality_from_seed(42) == personality_from_seed(42));
    CHECK(personality_from_seed(42) != personality_from_seed(43));
    Personality seeded_reference{.8779296875f, .509765625f, -.3759765625f};
    CHECK(personality_from_seed(42) == seeded_reference);
    for (uint32_t seed = 0; seed < 100; ++seed)
        for (auto value : personality_from_seed(seed))
            CHECK(value >= -1 && value <= 1);
    std::string replay_path = "brain-replay-" + std::to_string(brain.format()) + ".tmp";
    int calls = 0;
    double inference_seconds = 0;
    for (int species = 0; species < SpeciesCount; ++species) {
        World world, fork;
        reset(world, 1200 + species, species % 3, species, (species + 17) % 40,
              species % ArenaCount);
        BrainMemory memory{}, other_memory{};
        Personality personality =
            brain.format() == 3 ? personality_preset(species % PersonalityCount) : Personality{};
        BrainOutput rejected;
        CHECK(!brain.forward(observe(world, 0), memory, rejected, {0, 0, 1.01f}));
        Replay tape{world, {}};
        for (int t = 0; t < 90 && !world.terminal && !world.truncated; ++t) {
            auto bytes = snapshot(world);
            CHECK(restore(fork, bytes.data(), bytes.size()));
            other_memory = memory;
            auto original = hash(world);
            auto obs = observe(world, 0);
            auto start = std::chrono::steady_clock::now();
            auto action = brain.action(obs, memory, personality);
            inference_seconds +=
                std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            ++calls;
            CHECK(hash(world) == original);
            CHECK(obs.mask[action.ability] == 1);
            auto other = brain.action(observe(fork, 0), other_memory, personality);
            CHECK(action.mx == other.mx && action.my == other.my && action.ax == other.ax &&
                  action.ay == other.ay && action.ability == other.ability);
            CHECK(memory == other_memory);
            ReplayFrame frame;
            frame.actions = {action, scripted(world, 1)};
            replay_step(world, frame);
            replay_step(fork, frame);
            CHECK(hash(world) == hash(fork));
            frame.expected_hash = hash(world);
            tape.frames.push_back(frame);
        }
        CHECK(world.overflow == 0);
        CHECK(save_replay(tape, replay_path));
        Replay loaded;
        CHECK(load_replay(loaded, replay_path));
        World playback = loaded.initial;
        for (const auto &frame : loaded.frames)
            replay_step(playback, frame);
        CHECK(hash(playback) == hash(world));
        std::remove(replay_path.c_str());
    }
    std::cout << checks << " brain checks passed; " << calls << " decisions across all 40 species; "
              << inference_seconds * 1e6 / calls << " us/inference; "
              << (brain.format() == 2 ? 87523 : BrainParameters) << " parameters\n";
}

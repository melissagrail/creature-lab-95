#pragma once
// Campaign progression is separate from the deterministic duel and policy ABI.
#include "sim.hpp"
#include <array>
#include <filesystem>
#include <string>
#include <vector>
namespace creature::campaign {
constexpr int RegionCount = 8, SiteCount = 20, MapWidth = 64, MapHeight = 48;
constexpr std::array<int, 3> Starters{0, 1, 16};
enum SiteKind {
    Sanctuary,
    Conversation,
    Wild,
    Trial,
    Cache,
    Puzzle,
    Keeper,
    Gate,
    Expedition,
    Memory
};
struct Site {
    const char *name;
    int x, y, kind, species, prerequisite;
    const char *speaker;
    const char *text;
    const char *after;
};
struct Region {
    const char *name, *subtitle, *theme, *arrival, *epilogue;
    int arena, weather;
    std::array<int, 5> species;
    std::array<Site, SiteCount> sites;
};
extern const std::array<Region, RegionCount> Regions;
extern const std::array<std::array<uint8_t, MapWidth * MapHeight>, RegionCount> Maps;
struct Companion {
    int trust = 0, experience = 0, vitality = 1000, temperament = 0, charm = 0;
    uint32_t seed = 1;
};
struct State {
    uint32_t seed = 1;
    int region = 0, x = 8 * 256, y = 26 * 256, threads = 6, herbs = 3;
    int lead = 0, ending = 0, defeats = 0, victories = 0, play_seconds = 0;
    std::array<int, 3> party{0, -1, -1};
    std::array<Companion, SpeciesCount> companions{};
    std::array<int, RegionCount * SiteCount> cleared{};
    std::array<int, RegionCount> visits{};
    // A Lantern Walk saves between rooms, never halfway through a duel.
    int walk_region = -1, walk_depth = 0, walk_choice = 0;
    std::array<int, RegionCount> walks{};
    std::array<int, 4> lessons{};
};
struct Encounter {
    int region = 0, site = 0, rounds = 1, arena = 0, weather = 0;
    std::array<int, 5> enemies{};
    std::array<int, 5> styles{};
    uint32_t seed = 1;
    bool walk = false;
};
State new_journey(uint32_t seed, int starter);
int trust_needed(int species);
bool befriended(const State &, int species);
int collection_count(const State &);
int restored_count(const State &);
bool accessible(const State &, int region);
bool available(const State &, int region, int site);
int objective(const State &);
bool finish_story(State &, int site);
bool request_ready(const State &, int region);
bool solve_puzzle(State &, int site, const std::array<int, 3> &bells);
std::array<int, 3> puzzle_solution(int region);
bool offer_thread(State &, int species);
bool set_party(State &, int slot, int species);
void rest(State &);
bool craft_remedy(State &);
bool travel(State &, int region);
Encounter encounter(const State &, int site);
// Only called by the client after observing the actual duel/series result.
bool resolve(State &, const Encounter &, bool won, const std::array<int, 3> &vitality,
             bool withdrew = false);
int rank(const Companion &);
int rank_threshold(int rank);
struct Development {
    int arts, pace, capacity, recovery;
};
Development development(int rank);
int lesson_index(int site);
int lesson_count(int site);
std::string lesson_brief(const State &, int site);
bool charm_unlocked(const State &, int species, int charm);
bool begin_walk(State &);
bool rest_walk(State &);
bool leave_walk(State &);
Encounter walk_encounter(const State &, int choice);
int encounter_reward(const State &, const Encounter &);
bool trained_opponent(const State &, const Encounter &);
Action opponent_action(const World &, const Encounter &, int round);
// Short, deterministic keeper calls layered over the pilot's legal action.
// This controller can later be replaced by a learned response to the same guidance input.
Action companion_action(const World &, Action pilot, int temperament = 0);
bool beginner_assistance(const Body &);
bool friendship_ready(const State &);
void prepare_garden(World &, const Encounter &, int round);
void initialize_round(World &, const State &, const Encounter &, int round, int slot, int vitality,
                      int enemy_vitality);
bool passable(int region, int x, int y);
int ground(int region, int x, int y);
bool validate(const State &);
std::vector<uint8_t> serialize(const State &);
bool deserialize(State &, const uint8_t *, size_t);
bool save(const State &, const std::filesystem::path &, std::string &error);
bool load(State &, const std::filesystem::path &, std::string &error);
} // namespace creature::campaign

#include "creature/campaign.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
using namespace creature;
namespace c = creature::campaign;
static int checks = 0;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        ++checks;                                                                                  \
        if (!(x)) {                                                                                \
            std::cerr << "Campaign check failed at " << __LINE__ << ": " #x "\n";                  \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)
int main() {
    for (int region = 0; region < c::RegionCount; region++) {
        std::array<bool, c::MapWidth * c::MapHeight> visited{};
        std::queue<Vec> q;
        q.push({8, 26});
        visited[26 * c::MapWidth + 8] = true;
        while (!q.empty()) {
            auto p = q.front();
            q.pop();
            for (auto d : {Vec{1, 0}, Vec{-1, 0}, Vec{0, 1}, Vec{0, -1}}) {
                auto n = p + d;
                if (!c::passable(region, n.x, n.y))
                    continue;
                int i = n.y * c::MapWidth + n.x;
                if (!visited[i]) {
                    visited[i] = true;
                    q.push(n);
                }
            }
        }
        for (const auto &s : c::Regions[region].sites)
            CHECK(visited[s.y * c::MapWidth + s.x]);
    }
    for (int starter : {0, 6, 35})
        for (uint32_t seed : {1u, 42u, 0xffffffffu}) {
            auto state = c::new_journey(seed, starter);
            CHECK(c::validate(state));
            CHECK(state.party[0] == starter);
            CHECK(c::befriended(state, starter));
            CHECK(!c::travel(state, 7));
            CHECK(!c::finish_story(state, 16));
            auto bytes = c::serialize(state);
            auto copy = c::new_journey(777, 6);
            CHECK(c::deserialize(copy, bytes.data(), bytes.size()));
            CHECK(c::serialize(copy) == bytes);
            for (size_t i = 0; i < bytes.size(); i++) {
                auto bad = bytes;
                bad[i] ^= 1;
                auto before = c::serialize(copy);
                CHECK(!c::deserialize(copy, bad.data(), bad.size()));
                CHECK(c::serialize(copy) == before);
            }
            for (int rr = 0; rr < 8; ++rr) {
                CHECK(state.region == rr);
                CHECK(c::accessible(state, rr));
                for (int site : {2, 5, 7, 10, 13}) {
                    auto e = c::encounter(state, site);
                    int sp = c::Regions[rr].sites[site].species;
                    CHECK(!c::offer_thread(state, sp) || c::befriended(state, sp));
                    for (int count = 0; count < c::trust_needed(sp); count++)
                        CHECK(c::resolve(state, e, true, {750, 600, 500}));
                    CHECK(c::befriended(state, sp));
                    CHECK(c::validate(state));
                }
                auto e = c::encounter(state, 3);
                CHECK(!c::resolve(state, e, true, {1000, 1000, 1000}));
                CHECK(!c::solve_puzzle(state, 15, {-1, 0, 1}));
                CHECK(c::solve_puzzle(state, 15, c::puzzle_solution(rr)));
                int threads = state.threads;
                CHECK(c::solve_puzzle(state, 15, c::puzzle_solution(rr)));
                CHECK(state.threads == threads);
                for (int site : {1, 3, 4, 6, 9, 11, 12, 16}) {
                    CHECK(c::available(state, rr, site));
                    int k = c::Regions[rr].sites[site].kind;
                    if (k == c::Conversation)
                        CHECK(c::finish_story(state, site));
                    else {
                        e = c::encounter(state, site);
                        CHECK(e.rounds == (k == c::Keeper ? 3 : 2));
                        CHECK(c::resolve(state, e, true, {420, 0, 650}));
                    }
                    CHECK(c::validate(state));
                }
                CHECK(c::restored_count(state) == rr + 1);
                CHECK(c::objective(state) == 17);
                c::rest(state);
                CHECK(state.herbs >= 3);
                CHECK(c::validate(state));
                auto save = c::serialize(state);
                CHECK(c::deserialize(copy, save.data(), save.size()));
                CHECK(c::serialize(copy) == save);
                if (rr < 7)
                    CHECK(c::travel(state, rr + 1));
            }
            CHECK(c::collection_count(state) == 40);
            CHECK(c::set_party(state, 1, 39));
            CHECK(c::set_party(state, 2, 5));
            CHECK(c::set_party(state, 0, 39));
            CHECK(state.party[0] == 39);
            CHECK(c::validate(state));
            CHECK(!c::set_party(state, 3, 0));
            CHECK(!c::set_party(state, 0, 40));
            auto e = c::encounter(state, 18);
            CHECK(e.rounds == 5);
            int cash = state.threads;
            CHECK(c::resolve(state, e, false, {0, 0, 0}));
            CHECK(state.threads == cash);
            CHECK(state.x == 8 * 256);
            CHECK(state.companions[39].vitality == 1000);
            state.ending = 1;
            CHECK(c::validate(state));
            state.ending = 2;
            CHECK(c::validate(state));
            state.ending = 3;
            CHECK(!c::validate(state));
        }
    // First-contact recognition is awarded for a completed challenge, never retreat.
    {
        auto s = c::new_journey(31, 0);
        auto e = c::encounter(s, 5);
        int sp = c::Regions[0].sites[5].species;
        CHECK(c::resolve(s, e, false, {0, 0, 0}, true));
        CHECK(s.companions[sp].trust == 0);
        CHECK(c::resolve(s, e, false, {0, 0, 0}));
        CHECK(s.companions[sp].trust == 1);
        CHECK(c::resolve(s, e, false, {0, 0, 0}));
        CHECK(s.companions[sp].trust == 1);
        int cash = s.threads, herbs = s.herbs;
        CHECK(c::craft_remedy(s));
        CHECK(s.threads == cash - 2 && s.herbs == herbs + 1);
        s.threads = 1;
        CHECK(!c::craft_remedy(s));
        s.herbs = 99;
        s.threads = 10;
        CHECK(!c::craft_remedy(s));
    }
    // Optional requests are conditions, not dialogue rewards that can be farmed.
    {
        auto s = c::new_journey(42, 0);
        CHECK(!c::request_ready(s, 0));
        CHECK(!c::finish_story(s, 8));
        for (int id : {0, 1, 6})
            s.companions[id].trust = c::trust_needed(id);
        CHECK(c::set_party(s, 1, 1));
        s.companions[1].vitality = 123;
        CHECK(c::set_party(s, 1, 6));
        CHECK(s.companions[1].vitality == 123);
        CHECK(c::set_party(s, 1, 1));
        CHECK(s.companions[1].vitality == 123);
        CHECK(c::request_ready(s, 0));
        int cash = s.threads;
        CHECK(c::finish_story(s, 8));
        CHECK(s.threads == cash + 12);
        cash = s.threads;
        CHECK(c::finish_story(s, 8));
        CHECK(s.threads == cash);
        s.companions[0].experience = 120;
        CHECK(c::charm_unlocked(s, 0, 5));
        CHECK(!c::charm_unlocked(s, 0, 3));
        s.companions[0].charm = 5;
        CHECK(c::validate(s));
    }
    // Eight-room walks survive save/reload at every fork, consume campfires once,
    // block ordinary travel mid-walk, and preserve earned rewards on retreat.
    for (int rr = 0; rr < 8; ++rr) {
        auto s = c::new_journey(999, 0);
        for (int r = 0; r <= rr; ++r)
            for (int n : {1, 3, 4, 6, 9, 11, 12, 16})
                s.cleared[r * 20 + n] = 1;
        CHECK(c::travel(s, rr));
        CHECK(c::begin_walk(s));
        CHECK(!c::begin_walk(s));
        CHECK(!c::travel(s, 0));
        while (s.walk_region >= 0) {
            CHECK(c::validate(s));
            auto b = c::serialize(s);
            auto copy = c::new_journey(1, 6);
            CHECK(c::deserialize(copy, b.data(), b.size()));
            CHECK(c::serialize(copy) == b);
            int depth = s.walk_depth;
            if (depth == 2 || depth == 5) {
                s.threads = std::max(2, s.threads);
                CHECK(c::rest_walk(s));
                CHECK(s.walk_depth == depth + 1);
                CHECK(!c::rest_walk(s));
                continue;
            }
            s.walk_choice = depth % 2;
            auto e = c::walk_encounter(s, s.walk_choice);
            CHECK(e.walk && e.site == 18);
            CHECK(e.rounds == (depth == 7 ? 3 : depth % 2 ? 2 : 1));
            CHECK(c::trained_opponent(s, e) == bool(s.walk_choice));
            CHECK(c::encounter_reward(s, e) == e.rounds * (s.walk_choice ? 3 : 2));
            World w;
            reset(w, e.seed, e.weather, 0, e.enemies[0], e.arena);
            c::prepare_garden(w, e, 0);
            auto snap = snapshot(w);
            World restored;
            CHECK(restore(restored, snap.data(), snap.size()));
            CHECK(hash(restored) == hash(w));
            CHECK(c::resolve(s, e, true, {600, 0, 0}));
        }
        CHECK(s.walks[rr] == 1);
        CHECK(s.cleared[rr * 20 + 18]);
        CHECK(c::validate(s));
        CHECK(c::begin_walk(s));
        int cash = s.threads;
        CHECK(c::leave_walk(s));
        CHECK(s.threads == cash);
        CHECK(!c::leave_walk(s));
        CHECK(c::validate(s));
    }
    auto state = c::new_journey(42, 0);
    std::string error;
    auto path = std::filesystem::temp_directory_path() / "tinikami-campaign-test.tini";
    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(path.string() + ".bak", ec);
    CHECK(c::save(state, path, error));
    CHECK(c::finish_story(state, 1));
    CHECK(c::save(state, path, error));
    auto copy = c::new_journey(9, 6);
    CHECK(c::load(copy, path, error));
    CHECK(c::serialize(copy) == c::serialize(state));
    {
        std::ofstream corrupt(path, std::ios::binary);
        corrupt << "bad";
    }
    auto before = c::serialize(copy);
    CHECK(!c::load(copy, path, error));
    CHECK(c::serialize(copy) == before);
    CHECK(c::load(copy, path.string() + ".bak", error));
    CHECK(!copy.cleared[1]);
    CHECK(c::save(state, path, error));
    CHECK(c::load(copy, path.string() + ".bak", error));
    CHECK(!copy.cleared[1]); // Corruption did not replace backup.
    std::filesystem::remove(path, ec);
    std::filesystem::remove(path.string() + ".bak", ec);
    std::cout << checks
              << " campaign checks passed: eight connected regions, all species, both endings, "
                 "save corruption and backup.\n";
}

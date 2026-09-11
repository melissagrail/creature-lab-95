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
    // Party temperament steers the beginner helper, not just the neural pilot.
    {
        auto state = c::new_journey(91, 16);
        auto match = c::encounter(state, 2);
        World w;
        c::initialize_round(w, state, match, 0, 0, 1000, 1000);
        for (auto &o : w.obstacles) o.radius = 0;
        w.bodies[0].pos = {8*Q, 9*Q};
        w.bodies[1].pos = {10*Q, 9*Q};
        w.bodies[0].cooldown[0] = 20;
        auto timid = c::companion_action(w, {}, 2);
        auto bold = c::companion_action(w, {}, 1);
        CHECK(timid.mx < 0);
        CHECK(timid.mx != bold.mx || timid.my != bold.my);
        command(w, 0, Conserve);
        CHECK(c::companion_action(w, {}, 2).mx == 0);
        CHECK(c::companion_action(w, {}, 1).mx == 0);
    }
    // A keeper call is a legal controller action, not invulnerability or a forced hit.
    for (int starter : {0, 1, 16, 6, 35}) {
        auto state = c::new_journey(87, starter);
        auto encounter = c::encounter(state, 6);
        World retreat, still;
        c::initialize_round(retreat, state, encounter, 0, 0, 1000, 1000);
        for (auto &obstacle : retreat.obstacles)
            obstacle.radius = 0;
        retreat.bodies[0].pos = {13 * Q, 9 * Q};
        retreat.bodies[1].pos = {12 * Q, 9 * Q};
        still = retreat;
        int contact = 0;
        command(retreat, 0, Retreat);
        for (int tick = 0; tick < 72; tick += DecisionTicks) {
            Action burst{0, 0, Q, 0, tick == 0 ? 4 : 0};
            step(retreat, {c::companion_action(retreat, {}), burst});
            auto result = step(still, {Action{}, burst});
            contact += result.features[0].taken + result.features[0].shielded;
        }
        CHECK(retreat.bodies[0].hp == Roster[starter].hp);
        CHECK(contact > 0);
        command(retreat, 0, Conserve);
        auto action = c::companion_action(retreat, {Q, Q, Q, 0, 1});
        CHECK(action.mx == 0 && action.my == 0 && action.ability == 0);
        command(retreat, 0, Free);
        retreat.bodies[0].pace = 85;
        CHECK(c::companion_action(retreat, {Q, 0, Q, 0, 0}).mx == Q);
    }
    {
        auto state = c::new_journey(99, 0);
        for (int n = 0; n < 6; ++n) {
            auto e = c::encounter(state, 5);
            CHECK(c::resolve(state, e, true, {900, 0, 0}));
            if (n < 5)
                CHECK(c::collection_count(state) == 1);
        }
        CHECK(state.companions[1].trust == 1);
        CHECK(c::offer_thread(state, 1));
        CHECK(c::befriended(state, 1));
        CHECK(state.party[1] == 1);
        CHECK(c::validate(state));
    }
    {
        auto state = c::new_journey(44, 0);
        for (int n = 0; n < 6; ++n) {
            auto e = c::encounter(state, 5);
            CHECK(c::resolve(state, e, false, {0, 0, 0}, true));
            CHECK(!c::friendship_ready(state));
        }
        CHECK(state.defeats == 0);
        for (int n = 0; n < 6; ++n) {
            auto e = c::encounter(state, 5);
            CHECK(c::resolve(state, e, false, {0, 0, 0}));
        }
        CHECK(c::friendship_ready(state));
        CHECK(state.companions[1].trust == 1);
    }
    for (int starter : {0, 1, 16, 6, 35}) {
        auto s = c::new_journey(91, starter);
        CHECK(c::collection_count(s) == 1);
        for (int site : {2, 5, 7, 10, 13})
            if (c::Regions[0].sites[site].species != starter)
                CHECK(c::available(s, 0, site));
        CHECK(!c::friendship_ready(s));
        CHECK(c::finish_story(s, 1));
        for (int site : {3, 6}) {
            for (int j = 0; j < 4; ++j) {
                CHECK(c::available(s, 0, site));
                auto e = c::encounter(s, site);
                CHECK(e.rounds == 1);
                World w;
                c::initialize_round(w, s, e, 0, 0, 1000, 1000);
                CHECK(w.bodies[0].arts == 1);
                CHECK(w.bodies[1].arts == (site == 6 ? 8 : 1));
                CHECK(w.bodies[1].hp == Roster[w.bodies[1].species].hp);
                CHECK(w.bodies[0].capacity == 600);
                CHECK(w.objective == 0);
                CHECK(!w.vane_enabled);
                CHECK(c::resolve(s, e, true, {400, 0, 0}));
                CHECK(c::collection_count(s) == 1);
                CHECK(s.companions[starter].vitality == 1000);
                CHECK(bool(s.cleared[site]) == (j == 3));
                CHECK(c::validate(s));
                if (s.victories < 8)
                    for (int wild : {2, 5, 7, 10, 13})
                        if (c::Regions[0].sites[wild].species != starter)
                            CHECK(c::available(s, 0, wild));
            }
            if (site == 3)
                CHECK(c::finish_story(s, 4));
        }
        CHECK(s.victories == 8);
        CHECK(c::friendship_ready(s));
        CHECK(c::rank(s.companions[starter]) == 2);
        CHECK(c::available(s, 0, 2));
        auto e = c::encounter(s, 6);
        World w;
        c::initialize_round(w, s, e, 0, 0, 1000, 1000);
        CHECK(w.bodies[0].arts == 19);
        CHECK(w.bodies[0].capacity == 700);
        int xp = s.companions[starter].experience;
        CHECK(c::resolve(s, e, false, {0, 0, 0}, true));
        CHECK(s.companions[starter].experience == xp);
        // Upgrade a legacy V2 save without losing companions or completed lessons.
        auto old = c::serialize(s);
        old.erase(old.end() - 20, old.end() - 4);
        old[7] = '2';
        // Old relays awarded less XP than the newly credited lesson sequence.
        for (int n = 0; n < 4; ++n)
            old[68 + starter * 24 + n] = uint8_t(30u >> (n * 8));
        uint32_t hash = 2166136261u;
        for (size_t n = 0; n < old.size() - 4; ++n)
            hash = (hash ^ old[n]) * 16777619u;
        for (int n = 0; n < 4; ++n)
            old[old.size() - 4 + n] = uint8_t(hash >> (n * 8));
        c::State restored;
        CHECK(c::deserialize(restored, old.data(), old.size()));
        CHECK(restored.lessons[0] == 4 && restored.lessons[1] == 4);
        CHECK(restored.companions[starter].experience == xp);
    }
    for (int species = 0; species < 40; ++species)
        for (int rank = 1; rank <= 5; ++rank) {
            World w;
            reset(w, 77, 0, species, 0);
            auto d = c::development(rank);
            CHECK(configure_development(w, 0, d.arts, d.pace, d.capacity, d.recovery));
            auto obs = observe(w, 0);
            CHECK(obs.global[32] == float(d.arts) / 31);
            CHECK(obs.global[34] == float(d.capacity) / 1000);
            CHECK(travel_speed(w.bodies[0]) == Roster[species].speed * d.pace / 100);
            auto bytes = snapshot(w);
            World copy;
            CHECK(restore(copy, bytes.data(), bytes.size()));
            CHECK(hash(copy) == hash(w));
            for (int j = 0; j < 5; ++j)
                if (!(d.arts & (1 << j))) {
                    CHECK(!action_mask(w, 0)[j + 1]);
                    auto cpy = w;
                    step(cpy, {Action{0, 0, Q, 0, j + 1}, Action{}}, 1);
                    CHECK(cpy.bodies[0].move < 0);
                }
            for (int n = 0; n < 300; ++n)
                step(w, {Action{}, Action{}}, 1);
            CHECK(w.bodies[0].energy <= d.capacity);
            CHECK(!configure_development(w, 0, 31, 100, 1000, 100));
            for (int field = 0; field < 4; ++field) {
                auto bad = copy;
                auto &body = bad.bodies[0];
                if (field == 0)
                    body.arts = 32;
                if (field == 1)
                    body.pace = 49;
                if (field == 2)
                    body.capacity = 499;
                if (field == 3)
                    body.recovery_rate = 101;
                auto invalid = snapshot(bad);
                auto before = hash(copy);
                CHECK(!restore(copy, invalid.data(), invalid.size()));
                CHECK(hash(copy) == before);
            }
        }
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
    for (int starter : {0, 1, 16, 6, 35})
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
                auto e = c::encounter(state, 16);
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
                        CHECK(e.rounds == (rr == 0 ? 1 : k == c::Keeper ? 3 : 2));
                        do {
                            CHECK(c::resolve(state, e, true, {420, 0, 650}));
                        } while (!state.cleared[rr * 20 + site]);
                    }
                    CHECK(c::validate(state));
                }
                for (int site : {2, 5, 7, 10, 13}) {
                    auto e = c::encounter(state, site);
                    int sp = c::Regions[rr].sites[site].species;
                    CHECK(!c::offer_thread(state, sp) || c::befriended(state, sp));
                    for (int count = 0; count < c::trust_needed(sp); count++)
                        CHECK(c::resolve(state, e, true, {750, 600, 500}));
                    CHECK(c::befriended(state, sp));
                    CHECK(c::rank(state.companions[sp]) >= 1 + rr / 2);
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
        s.victories = 6;
        s.cleared[1] = s.cleared[3] = s.cleared[4] = s.cleared[6] = s.cleared[9] = 1;
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
        auto encounter = c::encounter(s, 2);
        World charm_world;
        for (int charm = 0; charm < 6; ++charm) {
            s.companions[0].charm = charm;
            c::initialize_round(charm_world, s, encounter, 0, 0, 1000, 1000);
            const int energy[] = {700, 595, 595, 595, 560, 455};
            CHECK(charm_world.bodies[0].energy == energy[charm]);
            auto snap = snapshot(charm_world);
            World copy;
            CHECK(restore(copy, snap.data(), snap.size()));
        }
        s.companions[0].charm = 0;
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
    // The beginner must close before planting a melee swing at a circling fox.
    // Previously Brambleback spent an entire duel whiffing at ~2,010 units.
    for (unsigned seed : {448456818u, 41u, 207u, 910u}) {
        auto beginner = c::new_journey(seed, 1);
        auto fight = c::encounter(beginner, 2);
        World duel;
        c::initialize_round(duel, beginner, fight, 0, 0, 1000, 1000);
        int casts = 0, hits = 0;
        while (!duel.terminal && !duel.truncated) {
            auto action = c::companion_action(duel, {});
            casts += action.ability != 0;
            auto result = step(duel, {action, c::opponent_action(duel, fight, 0)});
            hits += result.features[1].taken > 0;
        }
        CHECK(hits >= 5);
        CHECK(casts <= hits + 3);
        CHECK(duel.winner == 0);
    }
    std::cout << checks
              << " campaign checks passed: eight connected regions, all species, both endings, "
                 "save corruption and backup.\n";
}

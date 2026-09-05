#include "creature/sim.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
using namespace creature;
static int checks = 0;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        ++checks;                                                                                  \
        if (!(x)) {                                                                                \
            std::cerr << "environment failure " << __LINE__ << ": " #x "\n";                       \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)
static World scene(int species = 16) {
    World w;
    reset(w, 17, 0, species, 16, 2);
    w.surfaces = {};
    w.wind_base = {};
    w.vane_enabled = 0;
    w.objective = 0;
    w.bodies[0].pos = {4 * Q, 9 * Q};
    w.bodies[1].pos = {20 * Q, 9 * Q};
    return w;
}
static void idle(World &w, int ticks) {
    for (int i = 0; i < ticks; i++)
        step(w, {}, 1);
}
static void release(World &w, int slot, Vec aim = {Q, 0}) {
    const auto &m = move_for(w.bodies[0], slot);
    step(w, {{{0, 0, aim.x, aim.y, slot + 1}, {}}}, 1);
    idle(w, m.startup);
}
int main() {
    // Cast heading and amplitude, independent timers, opposition and expiry.
    for (int sp = 0; sp < SpeciesCount; sp++)
        for (int slot = 0; slot < 4; slot++) {
            auto &m = Moves[sp * 4 + slot];
            if (m.wind_strength) {
                World w = scene(sp);
                release(w, slot, {Q / 2, 0});
                CHECK(w.winds[0].force.x == m.wind_strength / 2 && w.winds[0].force.y == 0);
                CHECK(w.winds[0].life == m.wind_duration - 1);
                CHECK(observe(w, 0).moves[slot * MoveSize + 44] == float(m.wind_strength) / 24);
                w.wind_base = {3, -2};
                idle(w, m.wind_duration);
                CHECK(wind_vector(w).x == 3 && wind_vector(w).y == -2);
            }
            if (m.surface) {
                World w = scene(sp);
                release(w, slot);
                if (m.kind == Bolt)
                    idle(w, 90);
                bool found = false;
                for (auto &g : w.surfaces)
                    if (g.life && g.kind == m.surface) {
                        found = true;
                        CHECK(g.radius == m.surface_radius && g.owner == 0);
                    }
                CHECK(found);
                CHECK(observe(w, 0).entities[53 * EntitySize + 43] == 1);
            }
        }
    World w = scene();
    w.winds[0] = {{20, 0}, 10};
    w.winds[1] = {{-20, 0}, 5};
    CHECK(length(wind_vector(w)) == 0);
    idle(w, 5);
    CHECK(wind_vector(w).x == 20);
    idle(w, 5);
    CHECK(length(wind_vector(w)) == 0);
    w = scene();
    w.winds[0] = {{24, 24}, 10};
    w.winds[1] = {{24, 24}, 10};
    CHECK(length(wind_vector(w)) <= 24);
    w = scene();
    int startup = Moves[16 * 4 + 3].startup;
    step(w, {{{0, 0, Q, 0, 4}, {0, 0, -Q, 0, 4}}}, 1);
    idle(w, startup);
    CHECK(w.winds[0].life > 0 && w.winds[1].life > 0 && length(wind_vector(w)) == 0);
    // Uniform wind accelerates both projectile axes, at any position.
    w = scene();
    w.wind_base = {6, -4};
    for (int i = 0; i < 2; i++)
        w.projectiles[i] = {{(9 + i * 3) * Q, 7 * Q}, {200, 0}, {(9 + i * 3) * Q, 7 * Q}, 20, 0, 1};
    step(w, {}, 1);
    CHECK(w.projectiles[0].vel.x == 206 && w.projectiles[0].vel.y == -4);
    CHECK(w.projectiles[0].vel.x == w.projectiles[1].vel.x &&
          w.projectiles[0].vel.y == w.projectiles[1].vel.y);
    World head = scene(), tail = head;
    head.wind_base = {-20, 0};
    tail.wind_base = {20, 0};
    step(head, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    step(tail, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    CHECK(tail.bodies[0].vel.x > head.bodies[0].vel.x);
    // Every reaction is exercised by a travelling elemental projectile, not a direct helper.
    struct Reaction {
        int from, move, to;
    };
    for (auto r : {Reaction{Water, 24, Ice}, Reaction{Brush, 1, Fire}, Reaction{Water, 1, Steam},
                   Reaction{Ice, 1, Water}, Reaction{Water, 12, ChargedWater},
                   Reaction{Fire, 140, Steam}, Reaction{ChargedWater, 140, Water}}) {
        w = scene();
        w.surfaces[0] = {{12 * Q, 9 * Q}, 900, r.from, 600, -1, r.from, 0};
        w.projectiles[0] = {{12 * Q, 9 * Q}, {100, 0}, {12 * Q, 9 * Q}, 20, 0, r.move};
        step(w, {}, 1);
        CHECK(w.surfaces[0].kind == r.to);
        CHECK(w.surfaces[0].owner == 0);
        if (r.to == Ice || r.to == Steam || r.to == ChargedWater) {
            w.projectiles = {};
            idle(w, w.surfaces[0].effect_timer);
            CHECK(w.surfaces[0].kind == Water && w.surfaces[0].life > 0);
        }
        if (r.to == Fire) {
            w.projectiles = {};
            idle(w, w.surfaces[0].effect_timer);
            CHECK(w.surfaces[0].life == 0);
        }
    }
    // Beam and persistent field contact also alter ground.
    w = scene(28);
    w.surfaces[0] = {{8 * Q, 9 * Q}, 900, Water, 600, -1, Water, 0};
    release(w, 0);
    CHECK(w.surfaces[0].kind == Steam);
    w = scene(6);
    w.surfaces[0] = {{8 * Q, 9 * Q}, 900, Water, 600, -1, Water, 0};
    w.zones[0] = {{8 * Q, 9 * Q}, 60, 0, 26, 0, 0};
    step(w, {}, 1);
    CHECK(w.surfaces[0].kind == Ice);
    // Steam advects and drags projectiles; water extinguishes; ice preserves momentum.
    w = scene();
    w.wind_base = {10, 0};
    w.surfaces[0] = {{12 * Q, 9 * Q}, 900, Steam, 200, 0, Water, 90};
    w.projectiles[0] = {{12 * Q, 9 * Q}, {200, 0}, {12 * Q, 9 * Q}, 20, 0, 1};
    step(w, {}, 1);
    CHECK(w.surfaces[0].pos.x == 12 * Q + 20);
    CHECK(w.projectiles[0].vel.x == 210 * 92 / 100);
    w = scene();
    w.surfaces[0] = {w.bodies[0].pos, 900, Water, 200, -1, Water, 0};
    w.bodies[0].burn = 100;
    step(w, {}, 1);
    CHECK(w.bodies[0].burn == 0);
    CHECK(wet_at(w, w.bodies[0].pos));
    w = scene();
    w.surfaces[0] = {w.bodies[0].pos, 900, Ice, 200, -1, Water, 180};
    w.bodies[0].vel = {100, 0};
    step(w, {}, 1);
    CHECK(w.bodies[0].vel.x > 0);
    // Ground hazards hurt either team, including the creator. No self-damage farming credit.
    w = scene();
    w.bodies[1].pos = {6 * Q, 9 * Q};
    w.surfaces[0] = {{5 * Q, 9 * Q}, 1600, ChargedWater, 120, 0, Water, 100};
    int a = w.bodies[0].hp, b = w.bodies[1].hp;
    auto result = step(w, {}, 1);
    CHECK(w.bodies[0].hp == a - 3 && w.bodies[1].hp == b - 3);
    CHECK(result.features[0].dealt == 3 && result.features[0].taken == 3 &&
          result.features[1].taken == 3);
    // Vane capture, contest, cooldown, cast direction and leaving reset.
    w = scene();
    w.vane_enabled = 1;
    w.bodies[0].pos = w.vane_pos;
    idle(w, 44);
    CHECK(w.vane_capture[0] == 44);
    idle(w, 1);
    CHECK(w.vane_cooldown == 240 && w.winds[0].life == 150 && w.winds[0].force.x == 16);
    w = scene();
    w.vane_enabled = 1;
    w.bodies[0].pos = w.vane_pos - Vec{600, 0};
    w.bodies[1].pos = w.vane_pos + Vec{600, 0};
    idle(w, 50);
    CHECK(w.vane_capture[0] == 0 && w.vane_capture[1] == 0 && w.winds[0].life == 0);
    w.bodies[1].pos = {20 * Q, 9 * Q};
    idle(w, 5);
    CHECK(w.vane_capture[0] == 5);
    w.bodies[0].pos = {4 * Q, 9 * Q};
    idle(w, 1);
    CHECK(w.vane_capture[0] == 0);
    // Both contributions, transformed surfaces and objective progress survive a portable fork.
    w = scene();
    w.winds[0] = {{6, 8}, 99};
    w.winds[1] = {{-4, 0}, 65};
    w.surfaces[0] = {{12 * Q, 9 * Q}, 1800, Ice, 300, 1, Water, 170};
    w.vane_capture[0] = 12;
    auto bytes = snapshot(w);
    World restored;
    CHECK(restore(restored, bytes.data(), bytes.size()));
    CHECK(hash(w) == hash(restored));
    for (int i = 0; i < 150; i++) {
        step(w, {scripted(w, 0), scripted(w, 1)});
        step(restored, {scripted(restored, 0), scripted(restored, 1)});
        CHECK(hash(w) == hash(restored));
    }
    w = scene(35);
    for (auto &g : w.surfaces)
        g = {{12 * Q, 9 * Q}, 300, Brush, 600, -1, Brush, 0};
    release(w, 3);
    CHECK(w.overflow == 1);
    // Currents move planted bodies; freezing stops flow without discarding its vector.
    w = scene(2);
    w.surfaces[0] = {w.bodies[0].pos, 1800, Water, 600, 1, Water, 0, {20, 0}};
    auto start = w.bodies[0].pos;
    step(w, {{{0, 0, Q, 0, 1}, {}}}, 1);
    CHECK(w.bodies[0].pos.x > start.x && w.bodies[0].move == 8);
    w = scene(2);
    w.surfaces[0] = {w.bodies[0].pos, 1800, Ice, 600, 1, Water, 1, {20, 0}};
    start = w.bodies[0].pos;
    step(w, {}, 1);
    CHECK(w.bodies[0].pos.x == start.x && w.surfaces[0].kind == Water);
    step(w, {}, 1);
    CHECK(w.bodies[0].pos.x > start.x);
    auto current_save = snapshot(w);
    World current_fork;
    CHECK(restore(current_fork, current_save.data(), current_save.size()));
    CHECK(current_fork.surfaces[0].flow.x == 20);
    // Heavy melee shatters ice; mud can freeze, thaw, wash away or dry.
    w = scene(5);
    w.surfaces[0] = {w.bodies[0].pos + Vec{500, 0}, 1200, Ice, 600, 1, Water, 180};
    release(w, 0);
    CHECK(w.surfaces[0].kind == Water);
    for (auto r : {Reaction{Mud, 24, Ice}, Reaction{Mud, 140, Water}, Reaction{Mud, 1, Bare},
                   Reaction{Oil, 1, Fire}}) {
        w = scene();
        w.surfaces[0] = {{12 * Q, 9 * Q}, 900, r.from, 600, -1, r.from, 0};
        w.projectiles[0] = {{12 * Q, 9 * Q}, {100, 0}, {12 * Q, 9 * Q}, 20, 0, r.move};
        step(w, {}, 1);
        CHECK(w.surfaces[0].kind == r.to);
        if (r.from == Mud && r.to == Ice) {
            w.projectiles = {};
            idle(w, w.surfaces[0].effect_timer);
            CHECK(w.surfaces[0].kind == Mud);
        }
        if (r.to == Bare)
            CHECK(w.surfaces[0].life == 0);
    }
    // Fire spreads one graph edge per pulse, not through an entire chain in one tick.
    w = scene();
    w.surfaces[0] = {{10 * Q, 5 * Q}, 700, Fire, 300, 0, Bare, 120};
    w.surfaces[1] = {{11 * Q, 5 * Q}, 700, Oil, 600, -1, Oil, 0};
    w.surfaces[2] = {{12 * Q, 5 * Q}, 700, Brush, 600, -1, Brush, 0};
    step(w, {}, 1);
    CHECK(w.surfaces[1].kind == Fire && w.surfaces[2].kind == Brush);
    idle(w, 30);
    CHECK(w.surfaces[2].kind == Fire);
    w = scene();
    for (int j = 0; j < 3; j++)
        w.surfaces[j] = {w.bodies[0].pos, 1000, Fire, 200, 1, Bare, 120};
    int initial_hp = w.bodies[0].hp;
    step(w, {}, 1);
    CHECK(w.bodies[0].hp == initial_hp - 4);
    // Five owned patches form the creation budget; new creation replaces shortest remaining life.
    w = scene(35);
    for (int j = 0; j < 5; j++)
        w.surfaces[j] = {{12 * Q, 9 * Q}, 900, Oil, 600 - j, 0, Oil, 0};
    release(w, 3);
    int owned = 0;
    for (auto &g : w.surfaces)
        if (g.life && g.owner == 0)
            ++owned;
    CHECK(owned == 5 && w.overflow == 0 && w.surfaces[4].kind == Water);
    // New persisted fields reject malformed state atomically.
    World valid_world = scene(), destination = valid_world;
    auto original_hash = hash(destination);
    for (int fault = 0; fault < 5; fault++) {
        World malformed = valid_world;
        if (fault == 0)
            malformed.wind_base.x = 1000000;
        if (fault == 1)
            malformed.winds[0].life = 601;
        if (fault == 2)
            malformed.surfaces[0] = {{12 * Q, 9 * Q}, 1000, Water, 50, -2, Water, 0};
        if (fault == 3)
            malformed.vane_capture[0] = 45;
        if (fault == 4)
            malformed.surfaces[0] = {{12 * Q, 9 * Q}, 0, Water, 50, 0, Water, 0};
        auto data = snapshot(malformed);
        CHECK(!restore(destination, data.data(), data.size()));
        CHECK(hash(destination) == original_hash);
    }
    std::cout << checks << " environment checks passed\n";
}

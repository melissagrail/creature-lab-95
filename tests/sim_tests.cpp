#include "creature/replay.hpp"
#include "creature/sim.hpp"
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace creature;
static int checks = 0;
#define CHECK(x)                                                                                   \
    do {                                                                                           \
        checks++;                                                                                  \
        if (!(x)) {                                                                                \
            std::cerr << "FAIL line " << __LINE__ << ": " #x "\n";                                 \
            std::exit(1);                                                                          \
        }                                                                                          \
    } while (0)
static World duel(int species = 0, int enemy = 26) {
    World w;
    reset(w, 42, 0, species, enemy, 2);
    w.objective = 0;
    w.surfaces = {};
    w.vane_enabled = 0;
    w.bodies[0].pos = {8 * Q, 9 * Q};
    w.bodies[1].pos = {9 * Q, 9 * Q};
    return w;
}
static StepResult impact(World &w, int source = 0, int id = 1) {
    auto &t = w.bodies[1 - source];
    w.projectiles[0] = {t.pos, {0, 0}, t.pos, 2, source, id, 0, 0, 0, 0};
    return step(w,
                {{{0, 0, w.bodies[0].aim.x, w.bodies[0].aim.y, 0},
                  {0, 0, w.bodies[1].aim.x, w.bodies[1].aim.y, 0}}},
                1);
}
static void advance(World &w, int ticks) {
    for (int t = 0; t < ticks; t++)
        step(w, {}, 1);
}
static void cast(World &w, int slot) {
    Action a{0, 0, Q, 0, slot + 1};
    step(w, {a, {}}, 1);
}
int main() {
    const int D = Moves[1].damage;
    CHECK(Roster.size() == 40 && Moves.size() == 161);
    // Locomotion contracts across the roster: real yaw, directional speed, and braking.
    auto locomotion_world = [](int species) {
        World w;
        reset(w, 7, 0, species, 0, 2);
        w.objective = 0;
        w.surfaces = {};
        w.vane_enabled = 0;
        w.bodies[0].pos = {12 * Q, 9 * Q};
        w.bodies[1].pos = {22 * Q, 16 * Q};
        return w;
    };
    for (int sp = 0; sp < SpeciesCount; sp++) {
        auto &spec = Roster[sp];
        World turn = locomotion_world(sp);
        step(turn, {{{0, 0, -Q, 0, 0}, {}}}, 1);
        CHECK(turn.bodies[0].aim.x > 0 && turn.bodies[0].aim.y > 0);
        for (int t = 0; t < 180 / spec.turn_degrees + 4; t++)
            step(turn, {{{0, 0, -Q, 0, 0}, {}}}, 1);
        CHECK(turn.bodies[0].aim.x < -Q + 4 && std::abs(turn.bodies[0].aim.y) < 4);
        // Non-cardinal headings must settle exactly instead of oscillating from normalization loss.
        Vec target = unit({371, 833});
        turn = locomotion_world(sp);
        turn.bodies[0].aim = unit({-492, 762});
        for (int t = 0; t < 120; t++) {
            Vec before = turn.bodies[0].aim;
            step(turn, {{{0, 0, 371, 833, 0}, {}}}, 1);
            Vec after = turn.bodies[0].aim;
            double angle =
                std::atan2(double(int64_t(before.x) * after.y - int64_t(before.y) * after.x),
                           double(int64_t(before.x) * after.x + int64_t(before.y) * after.y));
            CHECK(std::abs(angle) * 180 / 3.141592653589793 < spec.turn_degrees + .3);
        }
        CHECK(turn.bodies[0].aim.x == target.x && turn.bodies[0].aim.y == target.y);
        int speeds[3] = {};
        for (int direction = 0; direction < 3; direction++) {
            World w = locomotion_world(sp);
            Action action{direction == 0   ? Q
                          : direction == 2 ? -Q
                                           : 0,
                          direction == 1 ? Q : 0, Q, 0, 0};
            for (int t = 0; t < 25; t++)
                step(w, {action, {}}, 1);
            speeds[direction] = length(w.bodies[0].vel);
            CHECK(w.bodies[0].aim.x == Q && w.bodies[0].aim.y == 0);
            for (int t = 0; t < 20; t++)
                step(w, {}, 1);
            CHECK(length(w.bodies[0].vel) == 0);
        }
        CHECK(speeds[0] >= speeds[1] && speeds[1] > speeds[2]);
        CHECK(std::abs(speeds[1] - spec.speed * (spec.strafe * 70 / 100) / 100) < 8);
        CHECK(std::abs(speeds[2] - spec.speed * (spec.backward * 70 / 100) / 100) < 8);
        for (int slot = 0; slot < 4; slot++) {
            const auto &m = Moves[sp * 4 + slot];
            World w = locomotion_world(sp);
            w.bodies[0].vel = {100, 0};
            Vec start = w.bodies[0].pos;
            step(w, {{{Q, 0, Q, 0, slot + 1}, {}}}, 1);
            if (m.move_start == 0)
                CHECK(length(w.bodies[0].pos - start) == 0);
            else
                CHECK(w.bodies[0].pos.x > start.x);
            Vec facing = w.bodies[0].aim;
            step(w, {{{0, 0, 0, Q, 0}, {}}}, 1);
            CHECK(w.bodies[0].aim.x == facing.x && w.bodies[0].aim.y == facing.y);
            auto o = observe(w, 0);
            CHECK(o.moves[slot * MoveSize + 37] == float(m.move_start) / 100);
            CHECK(o.self[56] == float(spec.turn_degrees * 30) / 600);
            CHECK(o.entities[43] == 1);
        }
        // Dodge direction comes from travel input, independent of facing and cast aim.
        World dodge = locomotion_world(sp);
        step(dodge, {{{0, -Q, Q, 0, 5}, {}}}, 1);
        CHECK(dodge.bodies[0].locked.y == -Q && dodge.bodies[0].aim.x == Q);
        Vec before = dodge.bodies[0].pos;
        step(dodge, {{{Q, 0, Q, 0, 0}, {}}}, 1);
        CHECK(dodge.bodies[0].pos.y < before.y && dodge.bodies[0].pos.x == before.x);
        CHECK(length(dodge.bodies[0].vel) == Moves[160].speed * spec.dodge_speed / 100);
        World fallback = locomotion_world(sp);
        step(fallback, {{{0, 0, Q, 0, 5}, {}}}, 1);
        CHECK(fallback.bodies[0].locked.y == Q && fallback.bodies[0].aim.x == Q);
        // Neither water traction nor held movement permits a planted cast to slide.
        for (int slot = 0; slot < 4; slot++)
            if (Moves[sp * 4 + slot].move_start == 0) {
                World wet = locomotion_world(sp);
                wet.wetness = 700;
                wet.bodies[0].vel = {180, 0};
                Vec origin = wet.bodies[0].pos;
                step(wet, {{{Q, 0, Q, 0, slot + 1}, {}}}, 1);
                CHECK(length(wet.bodies[0].pos - origin) == 0);
            }
    }
    // Rules 10: movement uses geometry and cooldowns, never ability energy.
    for (int sp = 0; sp < SpeciesCount; ++sp) {
        const auto &spec = Roster[sp];
        Body b;
        b.species = sp;
        b.energy = 500;
        CHECK(footwork_load(b) == 0 && energy_regen(b) == spec.regen);
        b.vel = {spec.speed, 0};
        CHECK(footwork_load(b) == 0 && energy_regen(b) == spec.regen);
        b.vel = {0, spec.speed * spec.strafe / 100};
        CHECK(footwork_load(b) == 100 && energy_regen(b) == spec.regen);
        b.vel = {0, spec.speed * spec.strafe / 500};
        CHECK(footwork_load(b) == 0);
        World rich = locomotion_world(sp), spent = rich;
        spent.bodies[0].energy = 0;
        for (int t = 0; t < 18; ++t) {
            step(rich, {{{0, Q, Q, 0, 0}, {}}}, 1);
            step(spent, {{{0, Q, Q, 0, 0}, {}}}, 1);
        }
        CHECK(length(spent.bodies[0].vel) == length(rich.bodies[0].vel));
        CHECK(rich.bodies[0].energy == 1000);
        CHECK(spent.bodies[0].energy == 18 * spec.regen);
        CHECK(observe(rich, 0).global[30] == float(energy_regen(rich.bodies[0])) / 12);
        World curve = locomotion_world(sp);
        curve.bodies[0].vel = {spec.speed, 0};
        step(curve, {{{0, Q, Q, 0, 0}, {}}}, 1);
        CHECK(std::abs(curve.bodies[0].vel.y) <=
              std::max(2, spec.speed * (5 + spec.turn_degrees) / 240) + 1);
        World burst = locomotion_world(sp);
        burst.bodies[0].vel = {spec.speed, 0};
        step(burst, {{{0, Q, Q, 0, 5}, {}}}, 1);
        step(burst, {{{0, Q, Q, 0, 0}, {}}}, 1);
        CHECK(burst.bodies[0].energy == 1000 && burst.bodies[0].vel.y > curve.bodies[0].vel.y);
        auto bytes = snapshot(rich);
        World copy;
        CHECK(restore(copy, bytes.data(), bytes.size()));
        CHECK(hash(copy) == hash(rich));
    }
    // Shared-energy opportunity costs: exact payment, finite reserve, and breathing windows.
    for (int sp = 0; sp < SpeciesCount; ++sp) {
        World energy = locomotion_world(sp);
        energy.bodies[0].energy = 500;
        World moving = energy;
        for (int t = 0; t < 10; ++t) {
            step(energy, {}, 1);
            step(moving, {{{Q, 0, Q, 0, 0}, {}}}, 1);
        }
        CHECK(energy.bodies[0].energy == std::min(1000, 500 + 10 * Roster[sp].regen));
        CHECK(energy.bodies[0].energy == moving.bodies[0].energy); // Forward travel is free.
        energy = locomotion_world(sp);
        auto &body = energy.bodies[0];
        const auto &m = move_for(body, 0);
        body.energy = m.cost - 1;
        CHECK(!action_mask(energy, 0)[1]);
        step(energy, {{{0, 0, Q, 0, 1}, {}}}, 1);
        CHECK(body.move == -1 && body.cooldown[0] == 0 && body.energy_delay == 0);
        body.energy = m.cost;
        CHECK(action_mask(energy, 0)[1]);
        cast(energy, 0);
        CHECK(body.energy == 0 && body.energy_delay == 23);
        CHECK(!action_mask(energy, 0)[5]); // Current cast commitment blocks dodge.
        World empty = locomotion_world(sp);
        empty.bodies[0].energy = 0;
        CHECK(action_mask(empty, 0)[5]);
        auto evade = step(empty, {{{0, Q, Q, 0, 5}, {}}}, 1);
        CHECK(evade.features[0].spent == 0);
        CHECK(empty.bodies[0].energy_delay == 0);
        CHECK(empty.bodies[0].cooldown[4] > 0);
        CHECK(!action_mask(empty, 0)[5]);
        CHECK(observe(energy, 1).entities[41] == 0);
        CHECK(observe(energy, 0).global[29] == 23.f / 24);
        auto bytes = snapshot(energy);
        World copy;
        CHECK(restore(copy, bytes.data(), bytes.size()));
        CHECK(hash(copy) == hash(energy));
    }
    {
        World energy = locomotion_world(0);
        cast(energy, 0);
        const int paid = energy.bodies[0].energy;
        advance(energy, 23);
        CHECK(energy.bodies[0].energy_delay == 0);
        CHECK(energy.bodies[0].energy == paid);
        step(energy, {}, 1);
        CHECK(energy.bodies[0].energy == paid + Roster[0].regen);
        CHECK(observe(energy, 0).global[30] == float(Roster[0].regen) / 12);
        World cannon = locomotion_world(2);
        cast(cannon, 0);
        int cannon_paid = cannon.bodies[0].energy;
        advance(cannon, 24);
        CHECK(cannon.bodies[0].energy_delay == 0 && phase(cannon.bodies[0]) == Startup);
        CHECK(cannon.bodies[0].energy == cannon_paid && energy_regen(cannon.bodies[0]) == 0);
        auto before = hash(energy);
        World bad = energy;
        bad.bodies[0].energy_delay = 25;
        auto bytes = snapshot(bad);
        CHECK(!restore(energy, bytes.data(), bytes.size()) && hash(energy) == before);
        World guard = locomotion_world(22);
        guard.bodies[0].energy = 500;
        guard.bodies[0].energy_delay = 10;
        guard.bodies[0].guard = 20;
        step(guard, {}, 1);
        CHECK(guard.bodies[0].energy == 507); // Guard passive bypasses the base lock.
    }
    // Every authored move: request, resource cost, telegraph, release, cooldown, snapshot.
    for (int species = 0; species < 40; species++)
        for (int slot = 0; slot < 5; slot++) {
            World w = duel(species);
            auto &m = move_for(w.bodies[0], slot);
            int id = move_id(w.bodies[0], slot);
            CHECK(action_mask(w, 0)[slot + 1]);
            cast(w, slot);
            CHECK(w.bodies[0].move == id);
            CHECK(w.bodies[0].cooldown[slot] == m.cooldown - 1);
            CHECK(w.bodies[0].energy == 1000 - m.cost ||
                  (species == 30 && w.bodies[0].counter % 3 == 0));
            CHECK(phase(w.bodies[0]) == Startup || m.startup == 1);
            advance(w, m.startup);
            bool release = false;
            for (auto &e : w.events)
                release = release || (e.kind == Released && e.move == id);
            CHECK(release);
            if (m.shots > 1)
                for (int n = 0; n < m.shots; n++) {
                    CHECK(w.projectiles[n].vel.x > 0);
                    CHECK(std::abs(w.projectiles[n].vel.x) > std::abs(w.projectiles[n].vel.y));
                }
            auto snap = snapshot(w);
            World restored;
            CHECK(restore(restored, snap.data(), snap.size()));
            CHECK(hash(w) == hash(restored));
            auto obs = observe(w, 0);
            CHECK(obs.moves[slot * MoveSize + 6] == float(m.damage) / 60);
            for (float v : obs.self)
                CHECK(std::isfinite(v));
            for (float v : obs.entities)
                CHECK(std::isfinite(v));
        }
    // Every damaging signature move must actually connect in at least one range probe.
    // This catches inert content which can start/release but has no executable hit behavior.
    for (int sp = 0; sp < 40; sp++)
        for (int slot = 0; slot < 4; slot++) {
            const auto &m = Moves[sp * 4 + slot];
            if (!m.damage)
                continue;
            bool connected = false;
            for (int distance : {900, 1600, 2500, 3500, 5000, 7000, 10000, 14000}) {
                World probe = duel(sp, 26);
                probe.bodies[0].pos = {3 * Q, 9 * Q};
                probe.bodies[1].pos = {3 * Q + distance, 9 * Q};
                int strength = (m.kind == Field || m.kind == Trap || m.kind == Turret)
                                   ? std::min(Q, distance * Q / std::max(1, m.range))
                                   : Q;
                auto result = step(probe, {{{0, 0, strength, 0, slot + 1}, {}}}, 1);
                int dealt = result.features[0].dealt;
                for (int t = 0; t < 360 && !probe.terminal; t++)
                    dealt += step(probe, {}, 1).features[0].dealt;
                connected = connected || dealt > 0;
            }
            if (!connected)
                std::cerr << "Inert move: " << Roster[sp].name << " / " << m.name << "\n";
            CHECK(connected);
        }
    // Independent worlds, content-stamped forks, every species in six arenas / three weathers.
    for (int s = 0; s < 40; s++) {
        World a, b;
        reset(a, 99 + s, s % 3, s, (s + 17) % 40, s % ArenaCount);
        b = a;
        for (int t = 0; t < 900 && !a.terminal && !a.truncated; t++) {
            std::array<Action, 2> acts{scripted(a, 0), scripted(a, 1, 1)};
            step(a, acts);
            step(b, acts);
            CHECK(hash(a) == hash(b));
            if (t % 13 == 0) {
                auto bytes = snapshot(a);
                CHECK(restore(b, bytes.data(), bytes.size()));
            }
        }
        CHECK(a.terminal || a.truncated);
        auto h = hash(a);
        CHECK(step(a, {}).ticks == 0);
        CHECK(hash(a) == h);
        CHECK(a.overflow == 0);
    }
    World a = duel(), b;
    auto bytes = snapshot(a);
    CHECK(restore(b, bytes.data(), bytes.size()));
    auto before = hash(b);
    bytes[8] ^= 1;
    CHECK(!restore(b, bytes.data(), bytes.size()));
    CHECK(hash(b) == before);
    CHECK(!restore(b, bytes.data(), bytes.size() - 1));
    // Ability INT_MIN must never overflow while converting to a slot.
    for (int i = 0; i < 100; i++)
        step(a, {{{INT_MAX, INT_MIN, INT_MIN, INT_MAX, INT_MIN}, {}}});
    CHECK(a.bodies[0].pos.y >= a.bodies[0].radius);
    // No early hit, one contact per action, attacks trade and double KO is a draw.
    a = duel();
    cast(a, 0);
    advance(a, Moves[0].startup - 1);
    CHECK(a.bodies[1].hp == Roster[26].hp);
    advance(a, 2);
    int damaged = a.bodies[1].hp;
    CHECK(damaged < Roster[26].hp);
    advance(a, 2);
    CHECK(a.bodies[1].hp == damaged);
    a = duel(14, 14);
    a.bodies[0].hp = a.bodies[1].hp = 10;
    step(a, {{{0, 0, Q, 0, 1}, {0, 0, -Q, 0, 1}}});
    advance(a, Moves[56].startup + 1);
    CHECK(a.terminal && a.winner == -1);
    // Minimum range, evasion, guard, shieldbreaking, CC lockout and cleanse.
    a = duel(2);
    cast(a, 0);
    advance(a, 25);
    CHECK(a.bodies[1].hp == Roster[26].hp);
    a = duel();
    a.bodies[1].move = 160;
    a.bodies[1].slot = 4;
    a.bodies[1].age = 1;
    CHECK(impact(a).features[1].dodged == 1);
    a = duel();
    a.bodies[1].shield = 30;
    a.bodies[1].shield_timer = 60;
    CHECK(impact(a).features[1].taken == 0);
    CHECK(a.bodies[1].shield == 30 - D);
    a = duel(19);
    a.bodies[1].shield = 30;
    a.bodies[1].shield_timer = 60;
    impact(a);
    CHECK(a.bodies[1].shield == 30 - 2 * D);
    a = duel();
    a.bodies[1].guard = 30;
    int guarded = impact(a).features[1].taken;
    CHECK(guarded < D);
    a = duel();
    a.bodies[1].cc_resist = 60;
    impact(a, 0, 5);
    CHECK(a.bodies[1].root == 0);
    a = duel(35);
    a.bodies[0].burn = 90;
    a.bodies[0].poison = 90;
    a.bodies[0].poison_stacks = 3;
    a.bodies[0].slow = 60;
    cast(a, 4);
    CHECK(a.bodies[0].burn == 0 && a.bodies[0].poison == 0 && a.bodies[0].slow == 0);
    a = duel();
    a.bodies[0].silence = 30;
    CHECK(!action_mask(a, 0)[1] && action_mask(a, 0)[5]);
    a.bodies[0].root = 30;
    CHECK(!action_mask(a, 0)[5]);
    // Destructible turret and trap, expiry, bank shot and return-hit lifetime.
    a = duel(20);
    cast(a, 1);
    advance(a, Moves[81].startup);
    CHECK(a.zones[0].hp == 45);
    a.bodies[1].pos = a.zones[0].pos;
    for (int k = 0; k < 4; k++) {
        a.projectiles[0] = {a.zones[0].pos, {}, a.zones[0].pos, 1, 1, 1, 0, 0, 0, 0};
        step(a, {}, 1);
    }
    CHECK(a.zones[0].life == 0);
    a = duel(10);
    a.zones[0] = {a.bodies[1].pos, 270, 0, 41, 0, 12};
    advance(a, 14);
    CHECK(a.bodies[1].hp == Roster[26].hp);
    advance(a, 2);
    CHECK(a.bodies[1].hp < Roster[26].hp && a.zones[0].life == 0);
    a = duel(13);
    a.projectiles[0] = {{24 * Q - 250, 5 * Q}, {500, 0}, {20 * Q, 5 * Q}, 20, 0, 52, 0, 0, 1, 0};
    step(a, {}, 1);
    CHECK(a.projectiles[0].life && a.projectiles[0].vel.x < 0 && a.projectiles[0].bounces == 0);
    a = duel(21);
    a.projectiles[0] = {{14 * Q, 9 * Q}, {370, 0}, {8 * Q, 9 * Q}, 24, 0, 84, 24, 2, 0, 1};
    step(a, {}, 1);
    CHECK(a.projectiles[0].returning == 2 && a.projectiles[0].vel.x < 0 &&
          a.projectiles[0].hit_mask == 0);
    // The 40 passive contracts, with controlled states rather than win-rate assertions.
    a = duel(0);
    a.bodies[1].burn = 30;
    CHECK(impact(a).features[0].dealt == D + 6); // Cinder
    a = duel(1);
    a.tick = 29;
    a.bodies[0].stationary = 30;
    step(a, {}, 1);
    CHECK(a.bodies[0].shield == 18); // Anchor
    a = duel(2);
    a.bodies[0].stationary = 90;
    CHECK(impact(a).features[0].dealt == D + 12); // Focus
    a = duel(3);
    a.wetness = 500;
    CHECK(impact(a).features[0].dealt == D * 115 / 100); // Conduit
    a = duel(4);
    for (int k = 0; k < 6; k++)
        impact(a, 0, 17);
    CHECK(a.bodies[1].poison_stacks == 5); // Venom
    a = duel(0, 5);
    CHECK(impact(a).features[1].taken == D * 80 / 100); // Bulwark
    a = duel(6);
    for (int k = 0; k < 3; k++)
        impact(a, 0, 24);
    CHECK(a.bodies[1].root > 0); // Frost
    a = duel(7);
    a.bodies[1].aim = {Q, 0};
    CHECK(impact(a).features[0].dealt == D + 9); // Backstab
    a = duel(0, 8);
    a.bodies[1].guard = 30;
    impact(a);
    CHECK(a.projectiles[0].owner == 1); // Mirror
    a = duel(9);
    a.tick = 29;
    a.wetness = 500;
    a.bodies[0].hp -= 10;
    step(a, {}, 1);
    CHECK(a.bodies[0].hp == Roster[9].hp - 8); // Rainborn
    a = duel(10);
    a.zones[0] = {a.bodies[1].pos, 100, 0, 41, 15, 12};
    CHECK(step(a, {}, 1).features[0].dealt == Moves[41].damage + 8); // Trapper
    a = duel(11);
    cast(a, 3);
    advance(a, Moves[47].startup);
    CHECK(a.bodies[0].haste > 50); // Renewal
    a = duel(12);
    a.bodies[0].meter = 100;
    CHECK(impact(a).features[0].dealt == D + 9 && a.bodies[0].meter == 0); // Ambush
    a = duel(13);
    CHECK(impact(a).features[0].dealt == D + 2); // Ricochet
    a = duel(14);
    a.bodies[1].wound = 90;
    CHECK(impact(a).features[0].dealt == D + 6); // Wounder
    a = duel(15);
    a.tick = 29;
    a.bodies[0].hp -= 10;
    a.zones[0] = {a.bodies[0].pos, 100, 0, 62, 1, 0};
    step(a, {}, 1);
    CHECK(a.bodies[0].hp == Roster[15].hp - 7); // Sanctuary
    a = duel(16);
    b = a;
    a.wind_base = {12, 0};
    step(a, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    step(b, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    CHECK(a.bodies[0].vel.x > b.bodies[0].vel.x); // Tailwind
    a = duel(17);
    a.bodies[0].hp = 40;
    CHECK(impact(a).features[0].dealt > D); // Berserk
    a = duel(18);
    a.bodies[0].last_slot = 1;
    cast(a, 0);
    CHECK(a.bodies[0].energy == 1000 - Moves[72].cost + 70); // Cadence refund bypasses rest lock
    a = duel(19);
    a.bodies[1].shield = 30;
    a.bodies[1].shield_timer = 60;
    impact(a);
    CHECK(a.bodies[1].shield == 30 - 2 * D); // Nullify
    a = duel(20);
    cast(a, 1);
    advance(a, Moves[81].startup);
    CHECK(a.zones[0].hp == 45); // Architect
    a = duel(21);
    CHECK(impact(a, 0, 84).features[0].dealt == Moves[84].damage + 2); // Returner
    a = duel(22);
    a.bodies[0].energy = 500;
    a.bodies[0].guard = 20;
    step(a, {}, 1);
    CHECK(a.bodies[0].energy == 500 + Roster[22].regen + 7); // Reservoir
    a = duel(23);
    a.bodies[0].hp -= 10;
    impact(a);
    CHECK(a.bodies[0].hp == Roster[23].hp - 10 + std::max(1, D / 5)); // Leech
    a = duel(24);
    a.bodies[0].counter = 2;
    CHECK(impact(a).features[0].dealt == D + 12); // Resonance
    a = duel(25);
    a.bodies[0].shield = 20;
    a.bodies[0].shield_timer = 20;
    CHECK(impact(a).features[0].dealt == D + 7); // Forge
    a = duel(26);
    a.bodies[0].meter = 1000;
    CHECK(impact(a).features[0].dealt == D + 10); // Skirmish
    a = duel(27);
    a.bodies[1].pos.x = 13 * Q;
    impact(a);
    CHECK(a.bodies[1].slow > 0); // Tether
    a = duel(28);
    a.bodies[0].meter = 100;
    CHECK(impact(a).features[0].dealt == D + 10); // Overheat
    a = duel(29);
    a.bodies[0].meter = 15;
    CHECK(impact(a).features[0].dealt == D + 15); // Retaliate
    a = duel(30);
    a.bodies[0].counter = 2;
    a.bodies[0].energy = 500;
    cast(a, 0);
    CHECK(a.bodies[0].energy == 500 - Moves[120].cost + 120); // Recycle
    a = duel(31);
    a.bodies[0].hp -= 20;
    a.zones[0] = {{4 * Q, 4 * Q}, 1, 0, 126, 1, 0};
    step(a, {}, 1);
    CHECK(a.bodies[0].hp == Roster[31].hp - 11); // Harvest
    a = duel(0, 32);
    a.zones[0] = {a.bodies[1].pos, 100, 1, 129, 1, 0};
    CHECK(impact(a).features[0].dealt == 0); // Cover
    a = duel(33);
    a.bodies[1].mark = 90;
    CHECK(impact(a).features[0].dealt == D + 5); // Hunter
    a = duel(34);
    impact(a);
    CHECK(a.bodies[1].wound > 0); // Corrode
    a = duel(35);
    a.bodies[0].burn = 90;
    cast(a, 4);
    CHECK(a.bodies[0].burn == 0); // Purify
    a = duel(36);
    a.bodies[0].meter = 60;
    CHECK(impact(a).features[0].dealt == D + 10); // Rhythm
    a = duel(37);
    b = a;
    a.zones[0] = {a.bodies[0].pos, 100, 0, 149, 1, 0};
    step(a, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    step(b, {{{Q, 0, Q, 0, 0}, {}}}, 1);
    CHECK(a.bodies[0].vel.x > b.bodies[0].vel.x); // Web
    a = duel(38);
    a.bodies[0].meter = 100;
    CHECK(impact(a).features[0].dealt == D + 16); // Magazine
    a = duel(39);
    a.bodies[0].meter = 100;
    CHECK(impact(a).features[0].dealt == D + 8); // Resolve
    // Public observation privacy and normalized shape contract.
    a = duel();
    auto o = observe(a, 0);
    a.bodies[1].energy = 7;
    a.bodies[1].cooldown[0] = 99;
    command(a, 1, Retreat);
    auto oo = observe(a, 0);
    CHECK(o.self == oo.self && o.history == oo.history);
    CHECK(oo.entities[41] == 7.f / 1000); // Energy is now public; cooldown/guidance remain private.
    oo.entities[41] = o.entities[41];
    CHECK(o.entities == oo.entities);
    // Objective rewards territory; contest blocks capture and timeout is separate.
    a = duel();
    a.objective = 1;
    a.bodies[0].pos = {12 * Q, 9 * Q};
    a.bodies[1].pos = {21 * Q, 9 * Q};
    advance(a, 631);
    CHECK(a.terminal && a.end_reason == 2 && a.winner == 0);
    a = duel();
    a.objective = 1;
    a.bodies[0].pos = {11 * Q, 9 * Q};
    a.bodies[1].pos = {13 * Q, 9 * Q};
    advance(a, 90);
    CHECK(a.bodies[0].control == 0 && a.bodies[1].control == 0);
    a = duel();
    a.tick = MaxTicks - 1;
    step(a, {});
    CHECK(a.truncated && !a.terminal);
    // Explicit capacity failure is deterministic and observable, never memory growth.
    a = duel();
    for (auto &p : a.projectiles)
        p = {{2 * Q, 2 * Q}, {}, {2 * Q, 2 * Q}, 60, 0, 1, 0, 0, 0, 0};
    cast(a, 1);
    advance(a, Moves[1].startup);
    CHECK(a.overflow == 1);
    // Seeded random action soak exercises arbitrary timing rather than only pilot choices.
    for (int sp = 0; sp < 40; sp++) {
        reset(a, 6000 + sp, sp % 3, sp, (sp + 11) % 40, sp % ArenaCount);
        uint32_t rng = sp + 1;
        auto next = [&]() {
            rng ^= rng << 13;
            rng ^= rng >> 17;
            rng ^= rng << 5;
            return rng;
        };
        for (int t = 0; t < 900 && !a.terminal && !a.truncated; t++) {
            std::array<Action, 2> acts;
            for (auto &act : acts)
                act = {int(next() % 2049) - 1024, int(next() % 2049) - 1024,
                       int(next() % 2049) - 1024, int(next() % 2049) - 1024, int(next() % 6)};
            step(a, acts);
            if (t % 11 == 0) {
                auto data = snapshot(a);
                CHECK(restore(b, data.data(), data.size()));
                CHECK(hash(a) == hash(b));
            }
        }
    }
    Replay tape;
    reset(tape.initial, 77, 2, 12, 27, 1);
    a = tape.initial;
    for (int k = 0; k < 50; k++) {
        ReplayFrame f;
        f.actions = {scripted(a, 0), scripted(a, 1)};
        if (k == 10) {
            f.guidance[0] = Retreat;
            f.feedback = 1;
        }
        replay_step(a, f);
        f.expected_hash = hash(a);
        tape.frames.push_back(f);
    }
    CHECK(save_replay(tape, "test-replay.tmp"));
    Replay loaded;
    CHECK(load_replay(loaded, "test-replay.tmp"));
    tape.frames.back().expected_hash ^= 1;
    CHECK(save_replay(tape, "test-replay.tmp"));
    CHECK(!load_replay(loaded, "test-replay.tmp"));
    std::remove("test-replay.tmp");
    reset(a, 77, 2, 12, 27, 1);
    for (int k = 0; k < 100; k++)
        step(a, {scripted(a, 0), scripted(a, 1)});
    std::cout << "Measured golden " << std::hex << hash(a) << std::dec << "\n";
    CHECK(hash(a) == 0x998dc4f56283125full);
    std::cout << checks << " alpha checks passed; golden " << std::hex << hash(a) << "\n";
}

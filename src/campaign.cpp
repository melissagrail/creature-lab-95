#include "creature/campaign.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <system_error>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace creature::campaign {
namespace {
uint32_t mix(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}
int index(int r, int s) { return r * SiteCount + s; }
bool valid_species(int s) { return s >= 0 && s < SpeciesCount; }
bool clear_path(const World &w, Vec start, Vec end, int padding) {
    Vec delta = end - start;
    int64_t squared = int64_t(delta.x) * delta.x + int64_t(delta.y) * delta.y;
    for (const auto &o : w.obstacles) {
        if (!o.radius)
            continue;
        Vec to = o.pos - start;
        int64_t along =
            std::clamp(int64_t(to.x) * delta.x + int64_t(to.y) * delta.y, int64_t(0), squared);
        Vec closest =
            squared ? start + Vec{int(delta.x * along / squared), int(delta.y * along / squared)}
                    : start;
        if (length(o.pos - closest) < o.radius + padding)
            return false;
    }
    return true;
}
void fill_party(State &s, int species) {
    if (!befriended(s, species))
        return;
    for (int r = 0; r < RegionCount; ++r)
        if (std::find(Regions[r].species.begin(), Regions[r].species.end(), species) !=
            Regions[r].species.end())
            s.companions[species].experience =
                std::max(s.companions[species].experience, rank_threshold(1 + r / 2));
    for (int id : s.party)
        if (id == species)
            return;
    for (int &id : s.party)
        if (id < 0) {
            id = species;
            return;
        }
}
uint32_t checksum(const uint8_t *p, size_t size) {
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < size; ++i)
        h = (h ^ p[i]) * 16777619u;
    return h;
}
template <class F> void fields(State &s, F f, bool legacy = false) {
    f(s.seed);
    f(s.region);
    f(s.x);
    f(s.y);
    f(s.threads);
    f(s.herbs);
    f(s.lead);
    f(s.ending);
    f(s.defeats);
    f(s.victories);
    f(s.play_seconds);
    for (auto &v : s.party)
        f(v);
    for (auto &c : s.companions) {
        f(c.trust);
        f(c.experience);
        f(c.vitality);
        f(c.temperament);
        f(c.charm);
        f(c.seed);
    }
    for (auto &v : s.cleared)
        f(v);
    for (auto &v : s.visits)
        f(v);
    f(s.walk_region);
    f(s.walk_depth);
    f(s.walk_choice);
    for (auto &v : s.walks)
        f(v);
    if (!legacy)
        for (auto &v : s.lessons)
            f(v);
}
} // namespace
State new_journey(uint32_t seed, int starter) {
    State s;
    s.seed = seed ? seed : 1;
    if (starter != 0 && starter != 1 && starter != 16 && starter != 6 && starter != 35)
        starter = 0;
    s.party = {starter, -1, -1};
    for (int i = 0; i < SpeciesCount; ++i) {
        s.companions[i].seed = mix(s.seed + uint32_t(i) * 0x9e3779b9u) | 1u;
        s.companions[i].temperament = int(s.companions[i].seed % 5);
    }
    s.companions[starter].trust = trust_needed(starter);
    s.visits[0] = 1;
    return s;
}
int trust_needed(int species) {
    for (const auto &r : Regions)
        for (int n = 0; n < 5; ++n)
            if (r.species[n] == species)
                return n < 1 ? 1 : n < 3 ? 2 : 3;
    return 3;
}
bool befriended(const State &s, int species) {
    return valid_species(species) && s.companions[species].trust >= trust_needed(species);
}
int collection_count(const State &s) {
    int count = 0;
    for (int i = 0; i < SpeciesCount; ++i)
        count += befriended(s, i);
    return count;
}
int restored_count(const State &s) {
    int n = 0;
    for (int r = 0; r < RegionCount; ++r)
        n += s.cleared[index(r, 16)] != 0;
    return n;
}
bool accessible(const State &s, int region) {
    return region >= 0 && region < RegionCount && (region == 0 || s.cleared[index(region - 1, 16)]);
}
bool available(const State &s, int region, int site) {
    if (!accessible(s, region) || site < 0 || site >= SiteCount)
        return false;
    // Hearthmere is an open teaching village. The journal suggests an order; only
    // the keeper and the road beyond still require the story to be completed.
    if (region == 0 && site != 16 && site != 17 && site != 18)
        return true;
    if (Regions[region].sites[site].kind == Wild &&
        s.companions[Regions[region].sites[site].species].trust > 0)
        return true;
    int prior = Regions[region].sites[site].prerequisite;
    return prior < 0 || s.cleared[index(region, prior)];
}
bool friendship_ready(const State &s) {
    return s.victories + s.defeats >= 6 || collection_count(s) > 1;
}
int objective(const State &s) {
    for (int site : {1, 3, 4, 6, 9, 11, 12, 16, 17})
        if (!s.cleared[index(s.region, site)])
            return site;
    return 18;
}
bool request_ready(const State &s, int region) {
    if (!accessible(s, region))
        return false;
    int locals = 0, met = 0, memories = 0;
    for (int id : Regions[region].species) {
        locals += befriended(s, id);
        met += s.companions[id].trust > 0;
    }
    for (int r = 0; r < RegionCount; ++r)
        memories += s.cleared[index(r, 19)] != 0;
    switch (region) {
    case 0:
        return collection_count(s) >= 3;
    case 1:
        return locals > 0 && s.cleared[index(1, 19)];
    case 2:
        return locals > 0 && s.cleared[index(2, 15)];
    case 3:
        return met == 5;
    case 4:
        return locals > 0 && s.cleared[index(4, 3)] && s.cleared[index(4, 6)];
    case 5:
        return memories >= 4;
    case 6:
        return restored_count(s) >= 6;
    case 7:
        return collection_count(s) == 40;
    }
    return false;
}
bool finish_story(State &s, int site) {
    if (!available(s, s.region, site))
        return false;
    int k = Regions[s.region].sites[site].kind;
    if (k != Conversation && k != Memory && k != Cache)
        return false;
    if (site == 8 && !request_ready(s, s.region))
        return false;
    auto &done = s.cleared[index(s.region, site)];
    if (!done) {
        s.threads = std::min(9999, s.threads + (site == 8 ? 12 : k == Cache ? 5 : 2));
        if (site == 8)
            for (int id : s.party)
                if (id >= 0)
                    s.companions[id].experience = std::min(99999, s.companions[id].experience + 60);
        if (k == Cache)
            s.herbs = std::min(99, s.herbs + 2);
        done = 1;
    }
    return true;
}
std::array<int, 3> puzzle_solution(int region) {
    static constexpr std::array<std::array<int, 3>, 8> notes{
        {{0, 2, 1}, {1, 0, 2}, {2, 1, 0}, {0, 1, 2}, {2, 0, 1}, {1, 2, 0}, {0, 1, 2}, {1, 0, 2}}};
    return notes[std::clamp(region, 0, 7)];
}
bool solve_puzzle(State &s, int site, const std::array<int, 3> &bells) {
    if (!available(s, s.region, site) || Regions[s.region].sites[site].kind != Puzzle ||
        bells != puzzle_solution(s.region))
        return false;
    auto &done = s.cleared[index(s.region, site)];
    if (!done) {
        done = 1;
        s.threads = std::min(9999, s.threads + 8);
    }
    return true;
}
bool offer_thread(State &s, int species) {
    if (!friendship_ready(s) || !valid_species(species) || befriended(s, species) ||
        s.threads < 3 || s.companions[species].trust < 1)
        return false;
    s.threads -= 3;
    ++s.companions[species].trust;
    fill_party(s, species);
    return true;
}
bool set_party(State &s, int slot, int species) {
    if (slot < 0 || slot >= 3 || !befriended(s, species))
        return false;
    int previous = s.party[slot];
    for (int n = 0; n < 3; ++n)
        if (s.party[n] == species)
            s.party[n] = previous;
    s.party[slot] = species;
    return true;
}
bool craft_remedy(State &s) {
    if (s.threads < 2 || s.herbs >= 99)
        return false;
    s.threads -= 2;
    ++s.herbs;
    return true;
}
void rest(State &s) {
    for (auto &c : s.companions)
        c.vitality = 1000;
    s.herbs = std::max(s.herbs, 3);
}
bool travel(State &s, int region) {
    if (!accessible(s, region) || s.walk_region >= 0)
        return false;
    s.region = region;
    s.x = 8 * 256;
    s.y = 26 * 256;
    s.visits[region] = 1;
    return true;
}
Encounter encounter(const State &s, int site) {
    Encounter e;
    e.region = s.region;
    e.site = site;
    const auto &r = Regions[s.region];
    const auto &n = r.sites[std::clamp(site, 0, SiteCount - 1)];
    e.arena = r.arena;
    e.weather = r.weather;
    e.seed =
        mix(s.seed ^ uint32_t(s.region * 100 + site) ^ uint32_t(s.victories + s.defeats) * 31337u);
    e.rounds = n.kind == Keeper ? 3 : n.kind == Expedition ? 5 : n.kind == Trial ? 2 : 1;
    for (int j = 0; j < 5; ++j) {
        e.enemies[j] = j == 0 ? n.species : r.species[(site + j * 2) % 5];
        e.styles[j] = (s.region + site + j) % 4;
    }
    if (s.region == 0 && n.kind == Trial) {
        e.rounds = 1;
        int step = s.lessons[lesson_index(site)];
        e.enemies[0] = site == 6 || site == 12 ? 10 : 35;
        e.styles[0] = step % 2;
    }
    if (s.region == 0 && n.kind == Keeper) {
        e.rounds = 1;
        e.enemies[0] = 10;
    }
    return e;
}
bool trained_opponent(const State &s, const Encounter &e) {
    if (e.region < 0 || e.region >= RegionCount || e.site < 0 || e.site >= SiteCount)
        return false;
    return (e.walk && s.walk_choice == 1) ||
           (e.region >= 3 && Regions[e.region].sites[e.site].kind == Keeper);
}
Action opponent_action(const World &w, const Encounter &e, int round) {
    auto action = scripted(w, 1, e.styles[std::clamp(round, 0, 4)]);
    if (e.region == 0 && !e.walk && e.site >= 0 && e.site < SiteCount) {
        bool lesson = Regions[0].sites[e.site].kind == Trial;
        const auto &b = w.bodies[1];
        // Quillrat teaches one unmistakable burst: approach, plant, charge, recover.
        // The keeper adds its ordinary ranged art between bursts.
        if (b.species == 10 && (lesson || e.site == 16)) {
            Vec toward = w.bodies[0].pos - b.pos;
            int distance = length(toward);
            Vec direction = unit(toward);
            action = {direction.x, direction.y, direction.x, direction.y, 0};
            if (distance < 3000 || b.move >= 0 || w.tick % 210 >= 150)
                action.mx = action.my = 0;
            Vec from_centre = b.pos - Vec{12 * Q, 9 * Q};
            bool yield_lotus = lesson && w.objective && length(from_centre) < 3000;
            if (yield_lotus && b.move < 0) {
                Vec away = length(from_centre) ? unit(from_centre) : Vec{Q, 0};
                action.mx = away.x;
                action.my = away.y;
            }
            auto mask = action_mask(w, 1);
            if (!yield_lotus && distance < 3900 && mask[4])
                action.ability = 4;
            else if (!lesson && distance >= 3900 && w.tick % 120 < DecisionTicks && mask[1])
                action.ability = 1;
            return action;
        }
        if (!lesson) {
            // Local wild partners leave a readable gap between commitments, too.
            if (w.tick % 75 >= DecisionTicks)
                action.ability = 0;
            return action;
        }
        // These are teaching partners. An attack opportunity every three seconds leaves
        // a clear observation-response-recovery rhythm; the actual art uses normal physics.
        if (action.ability == 5) {
            action.ability = 0;
            action.mx = action.my = 0;
        }
        int cadence = e.site <= 6 ? 90 : 75;
        if (w.tick % cadence >= DecisionTicks)
            action.ability = 0;
        if (w.tick % cadence >= cadence / 2)
            action.mx = action.my = 0;
        // Demonstrate the new objective by leaving the lotus to the learner. Keepers
        // contest it normally; a stationary lesson partner must not win by camping it.
        Vec from_centre = w.bodies[1].pos - Vec{12 * Q, 9 * Q};
        if (w.objective && length(from_centre) < 3000 && w.bodies[1].move < 0) {
            Vec away = length(from_centre) ? unit(from_centre) : Vec{Q, 0};
            action.mx = away.x;
            action.my = away.y;
        }
    }
    return action;
}
bool beginner_assistance(const Body &b) { return b.pace <= 75; }
Action companion_action(const World &w, Action pilot) {
    const auto &b = w.bodies[0];
    // Beginners approach, face and plant for an affordable attack. They do not
    // automatically read the slow burst: learning when to call retreat is the lesson.
    int directive = b.guidance_age < 90 ? b.guidance : Free;
    if (directive == Free && beginner_assistance(b))
        directive = Attack;
    if (directive == Free)
        return pilot;
    if (b.move >= 0)
        return {0, 0, b.locked.x, b.locked.y, 0};
    Vec delta = w.bodies[1].pos - b.pos;
    Vec aim = unit(delta);
    if (directive == Retreat) {
        // Turn and run forward: backing away at reverse speed is deliberately slower.
        // At an edge choose the safest reachable heading, rather than pushing into it.
        Vec best{};
        int best_score = -100000000;
        for (Vec candidate : {Vec{Q, 0}, Vec{-Q, 0}, Vec{0, Q}, Vec{0, -Q}, Vec{724, 724},
                              Vec{-724, 724}, Vec{724, -724}, Vec{-724, -724}}) {
            Vec end = b.pos + scale(candidate, 2200);
            if (end.x < b.radius || end.x > 24 * Q - b.radius || end.y < b.radius ||
                end.y > 18 * Q - b.radius)
                continue;
            bool blocked = false;
            for (const auto &o : w.obstacles)
                for (int sample = 1; sample <= 3; ++sample)
                    if (o.radius && length(b.pos + scale(end - b.pos, sample, 3) - o.pos) <
                                        o.radius + b.radius + 250)
                        blocked = true;
            if (blocked)
                continue;
            int score = length(end - w.bodies[1].pos);
            if (score > best_score) {
                best_score = score;
                best = candidate;
            }
        }
        return {best.x, best.y, best.x, best.y, 0};
    }
    if (directive == Conserve)
        return {0, 0, aim.x, aim.y, 0};
    auto mask = action_mask(w, 0);
    int choice = 0, reach = 1000;
    bool obstructed = !clear_path(w, b.pos, w.bodies[1].pos, 150);
    // Request the first affordable offensive art whose actual reach covers the target.
    for (int slot = 0; slot < 4; ++slot) {
        const auto &m = move_for(b, slot);
        if (!(b.arts & (1 << slot)) || m.damage <= 0)
            continue;
        int range = m.kind == Nova ? m.radius : m.kind == Melee ? m.range / 2 + m.radius : m.range;
        if (m.kind == Field || m.kind == Trap)
            range += m.radius;
        if (m.kind == Lunge)
            range += m.speed * m.active;
        reach = std::max(reach, range + w.bodies[1].radius - 200);
        if (!choice && !obstructed && mask[slot + 1] &&
            length(delta) >= m.min_range + (m.min_range ? w.bodies[1].radius : 0) &&
            length(delta) <= range + w.bodies[1].radius - 100) {
            choice = slot + 1;
            int lead = m.startup + (m.kind == Bolt && m.speed ? length(delta) / m.speed : 0);
            aim = unit(delta + scale(w.bodies[1].vel, std::min(45, lead), 1));
            if (m.kind == Field || m.kind == Trap || m.kind == Turret)
                aim = scale(aim, std::min(Q, length(delta) * Q / std::max(1, m.range)));
        }
    }
    if (choice)
        return {0, 0, aim.x, aim.y,
                int64_t(unit(aim).x) * b.aim.x + int64_t(unit(aim).y) * b.aim.y >= Q * Q * 97 / 100
                    ? choice
                    : 0};
    if (length(delta) > reach || obstructed) {
        Vec best{};
        int score = -100000000;
        for (Vec candidate : {unit(delta), Vec{Q, 0}, Vec{-Q, 0}, Vec{0, Q}, Vec{0, -Q},
                              Vec{724, 724}, Vec{-724, 724}, Vec{724, -724}, Vec{-724, -724}}) {
            Vec end = b.pos + scale(candidate, 900);
            if (end.x < b.radius || end.x > 24 * Q - b.radius || end.y < b.radius ||
                end.y > 18 * Q - b.radius || !clear_path(w, b.pos, end, b.radius + 50))
                continue;
            int merit = -length(w.bodies[1].pos - end);
            if (merit > score) {
                score = merit;
                best = candidate;
            }
        }
        return {best.x, best.y, best.x, best.y, 0};
    }
    return {0, 0, aim.x, aim.y, 0};
}
int encounter_reward(const State &s, const Encounter &e) {
    return e.rounds * (e.walk && s.walk_choice == 1 ? 3 : 2);
}
bool resolve(State &s, const Encounter &e, bool won, const std::array<int, 3> &vitality,
             bool withdrew) {
    if (e.region != s.region || !available(s, e.region, e.site))
        return false;
    if (e.walk && (s.walk_region != s.region || e.site != 18 || s.walk_depth >= 8))
        return false;
    const auto &n = Regions[s.region].sites[e.site];
    if (n.kind != Wild && n.kind != Trial && n.kind != Keeper && n.kind != Expedition)
        return false;
    for (int j = 0; j < 3; ++j)
        if (s.party[j] >= 0) {
            auto &c = s.companions[s.party[j]];
            c.vitality = std::clamp(vitality[j], 0, 1000);
            c.experience = std::min(99999, c.experience + (withdrew ? 0 : won ? e.rounds * 15 : 5));
        }
    if (won) {
        ++s.victories;
        s.threads = std::min(9999, s.threads + encounter_reward(s, e));
        if (e.walk) {
            ++s.walk_depth;
            if (s.walk_depth >= 8) {
                s.walk_region = -1;
                s.walk_depth = 0;
                s.walks[e.region] = std::min(9999, s.walks[e.region] + 1);
                s.cleared[index(e.region, 18)] = 1;
                s.threads = std::min(9999, s.threads + 12);
                rest(s);
            }
        } else if (e.region == 0 && n.kind == Trial) {
            int &progress = s.lessons[lesson_index(e.site)];
            progress = std::min(lesson_count(e.site), progress + 1);
            if (progress >= lesson_count(e.site))
                s.cleared[e.site] = 1;
            // Lessons are separate, readable duels; each ends with a free recovery.
            rest(s);
        } else
            s.cleared[index(e.region, e.site)] = 1;
        if (n.kind == Wild && friendship_ready(s)) {
            auto &c = s.companions[n.species];
            c.trust = std::min(trust_needed(n.species), c.trust + 1);
            fill_party(s, n.species);
        }
        if (n.kind == Keeper)
            rest(s);
    } else {
        if (!withdrew)
            ++s.defeats;
        // A first completed friendly challenge builds recognition even in defeat.
        // Retreat is not a completed meeting; it never grants acquisition progress.
        if (!withdrew && n.kind == Wild && friendship_ready(s) &&
            s.companions[n.species].trust == 0) {
            s.companions[n.species].trust = 1;
            fill_party(s, n.species);
        }
        s.walk_region = -1;
        s.walk_depth = 0;
        rest(s);
        s.x = 8 * 256;
        s.y = 26 * 256;
    }
    return true;
}
bool charm_unlocked(const State &s, int species, int charm) {
    if (!befriended(s, species) || charm < 0 || charm > 5)
        return false;
    if (charm == 0)
        return true;
    if (rank(s.companions[species]) < 2)
        return false;
    if (charm < 3)
        return true;
    return s.cleared[index(charm == 3 ? 1 : charm == 4 ? 5 : 0, 8)] != 0;
}
bool begin_walk(State &s) {
    if (s.walk_region >= 0 || !available(s, s.region, 18))
        return false;
    s.walk_region = s.region;
    s.walk_depth = 0;
    s.walk_choice = 0;
    return true;
}
bool rest_walk(State &s) {
    if (s.walk_region != s.region || (s.walk_depth != 2 && s.walk_depth != 5) || s.threads < 2)
        return false;
    s.threads -= 2;
    for (int id : s.party)
        if (id >= 0)
            s.companions[id].vitality = std::min(1000, s.companions[id].vitality + 350);
    s.herbs = std::min(99, s.herbs + 1);
    ++s.walk_depth;
    return true;
}
bool leave_walk(State &s) {
    if (s.walk_region < 0)
        return false;
    s.walk_region = -1;
    s.walk_depth = 0;
    rest(s);
    s.x = 8 * 256;
    s.y = 26 * 256;
    return true;
}
Encounter walk_encounter(const State &s, int choice) {
    Encounter e = encounter(s, 18);
    e.walk = true;
    choice = std::clamp(choice, 0, 1);
    e.seed = mix(e.seed ^ uint32_t(s.walk_depth * 1237 + choice * 8191 + s.walks[s.region] * 7219));
    e.rounds = s.walk_depth == 7 ? 3 : choice ? 2 : 1;
    e.arena = choice ? (Regions[s.region].arena + s.walk_depth + 1) % 6 : Regions[s.region].arena;
    e.weather = choice ? (s.region + s.walk_depth) % 3 : Regions[s.region].weather;
    for (int j = 0; j < 5; ++j) {
        e.enemies[j] = Regions[s.region].species[(s.walk_depth + choice * 2 + j * 2) % 5];
        e.styles[j] = (s.walk_depth + j + choice) % 4;
    }
    return e;
}
void initialize_round(World &w, const State &s, const Encounter &e, int round, int slot,
                      int vitality, int enemy_vitality) {
    int own = s.party[slot];
    reset(w, e.seed + uint32_t(round * 97 + slot * 13), e.weather, own, e.enemies[round], e.arena);
    w.bodies[0].hp = std::max(1, Roster[own].hp * std::clamp(vitality, 0, 1000) / 1000);
    w.bodies[1].hp =
        std::max(1, Roster[e.enemies[round]].hp * std::clamp(enemy_vitality, 0, 1000) / 1000);
    prepare_garden(w, e, round);
    auto own_growth = development(rank(s.companions[own]));
    int foe_rank = std::clamp(1 + e.region, 1, 5);
    if (e.region == 0 && e.site >= 9)
        foe_rank = 2;
    if (e.walk)
        foe_rank = std::max(3, foe_rank);
    auto foe_growth = development(foe_rank);
    if (e.region == 0 && !e.walk && e.enemies[round] == 10 &&
        (Regions[0].sites[e.site].kind == Trial || e.site == 16))
        foe_growth = {e.site == 16 ? 9 : 8, 65, 700, 70};
    configure_development(w, 0, own_growth.arts, own_growth.pace, own_growth.capacity,
                          own_growth.recovery);
    configure_development(w, 1, foe_growth.arts, foe_growth.pace, foe_growth.capacity,
                          foe_growth.recovery);
    if (e.region == 0 && !e.walk) {
        // One spatial idea per garden, without early weather or layered hazards.
        w.surfaces = {};
        w.wind_base = {};
        w.winds = {};
        w.rain = w.wetness = 0;
        w.vane_enabled = 0;
        w.objective = e.site == 9 || e.site == 12;
        for (auto &o : w.obstacles)
            o.radius = 0;
        w.bodies[0].pos = {8 * Q, 9 * Q};
        w.bodies[1].pos = {16 * Q, 9 * Q};
        int teacher = lesson_index(e.site);
        if (teacher >= 0) {
            int lesson = s.lessons[teacher] % lesson_count(e.site);
            // Same spatial vocabulary, different approach angles within each lesson set.
            int offset = lesson == 1 ? -2 : lesson == 2 ? 2 : 0;
            w.bodies[0].pos.y += offset * Q;
            w.bodies[1].pos.y -= offset * Q;
        }
        if (e.site == 3) {
            // Two stepping stones leave an open central lane and distinct flanks.
            w.obstacles[0] = {{12 * Q, 5 * Q}, 900};
            w.obstacles[1] = {{12 * Q, 13 * Q}, 900};
        } else if (e.site == 6) {
            // Charge lesson: generous escape space, with a stone behind each start.
            w.obstacles[0] = {{4 * Q, 9 * Q}, 900};
            w.obstacles[1] = {{20 * Q, 9 * Q}, 900};
        } else if (e.site == 9) {
            // A fork around two rocks leads to the lotus; each flank has soft ground.
            w.obstacles[0] = {{12 * Q, 5 * Q}, 1100};
            w.obstacles[1] = {{12 * Q, 13 * Q}, 1100};
            w.surfaces[0] = {{8 * Q, 13 * Q}, 1500, Mud, MaxTicks, -1, Mud, 0, {}};
            w.surfaces[1] = {{16 * Q, 5 * Q}, 1500, Mud, MaxTicks, -1, Mud, 0, {}};
        } else if (e.site == 12 || e.site == 16) {
            // Broad wet corners leave a dry cross through the charge arena.
            for (int j = 0; j < 4; ++j)
                w.surfaces[j] = {{(j % 2 ? 18 : 6) * Q, (j / 2 ? 14 : 4) * Q},
                                 1800,
                                 Water,
                                 MaxTicks,
                                 -1,
                                 Water,
                                 0,
                                 {}};
        } else {
            int offset = e.site % 3 - 1;
            w.obstacles[0] = {{12 * Q, (5 + offset) * Q}, 1000};
            w.obstacles[1] = {{12 * Q, (13 + offset) * Q}, 1000};
        }
    }
    int charm = s.companions[own].charm;
    if (charm == 1) {
        w.bodies[0].shield = 12;
        w.bodies[0].shield_timer = 450;
        w.bodies[0].energy = 850;
    }
    if (charm == 2) {
        w.bodies[0].haste = 90;
        w.bodies[0].energy = 850;
    }
    if (charm == 3)
        w.bodies[0].energy = 850;
    if (charm == 4) {
        w.bodies[0].cc_resist = 150;
        w.bodies[0].energy = 800;
    }
    if (charm == 5) {
        w.bodies[0].shield = 24;
        w.bodies[0].shield_timer = 450;
        w.bodies[0].energy = 650;
    }
    if (charm)
        w.bodies[0].energy = w.bodies[0].energy * own_growth.capacity / 1000;
}
void prepare_garden(World &w, const Encounter &e, int round) {
    // Scenario setup uses ordinary observable engine terrain, not special-case damage rules.
    int surface = e.region == 0   ? Brush
                  : e.region == 1 ? Water
                  : e.region == 2 ? Bare
                  : e.region == 3 ? Brush
                  : e.region == 4 ? Oil
                  : e.region == 5 ? Ice
                  : e.region == 6 ? Water
                                  : Ice;
    int offset = (e.site + round) % 3 - 1;
    if (surface != Bare) {
        w.surfaces[6] = {{(10 + offset) * Q, 6 * Q},       1200, surface, MaxTicks, -1,
                         surface == Ice ? Water : surface, 0,    {12, 0}};
        w.surfaces[7] = {{(14 - offset) * Q, 12 * Q},      1200, surface, MaxTicks, -1,
                         surface == Ice ? Water : surface, 0,    {-12, 0}};
    }
    if (e.region == 2 || e.region == 7) {
        int power = e.region == 2 ? 14 : 10;
        w.wind_base = round % 2 ? Vec{-power, 5} : Vec{power, -5};
    }
}
int rank_threshold(int r) {
    static const int thresholds[] = {0, 0, 120, 300, 600, 1000};
    return thresholds[std::clamp(r, 1, 5)];
}
int rank(const Companion &c) {
    int r = 1;
    while (r < 5 && c.experience >= rank_threshold(r + 1))
        ++r;
    return r;
}
Development development(int r) {
    static const Development stages[] = {{1, 65, 600, 60},
                                         {19, 75, 700, 70},
                                         {23, 85, 800, 80},
                                         {31, 95, 900, 90},
                                         {31, 100, 1000, 100}};
    return stages[std::clamp(r, 1, 5) - 1];
}
int lesson_index(int site) {
    return site == 3 ? 0 : site == 6 ? 1 : site == 9 ? 2 : site == 12 ? 3 : -1;
}
int lesson_count(int site) { return site == 3 || site == 6 ? 4 : 3; }
std::string lesson_brief(const State &s, int site) {
    int i = lesson_index(site);
    if (s.region != 0 || i < 0)
        return "";
    static const char *tips[] = {
        "CALL AND ANSWER. Your spirit pilots itself. Press F to ask for an attack, G to fall "
        "back, or C to rest. Calls last three combat seconds. They never cancel an art already "
        "committed. The two stones offer cover; the middle lane is open.",
        "READ THE BIG CIRCLE. Quillrat plants its feet and charges a wide burst for two combat "
        "seconds. Call FALL BACK [G] while the gold circle fills. When the burst ends, call "
        "ATTACK [F] during its long recovery. Start your retreat before it flashes.",
        "CHOOSE A ROUTE. Two stones split the approach to the lotus. Mud slows a flank. Your "
        "second art and dodge awaken at bond 2, after eight wins. You can win this lesson by "
        "holding the lotus or by outlasting your partner.",
        "PUT IT TOGETHER. Quillrat's burst threatens the dry crossing. Wet corners offer another "
        "route. Call a retreat before the burst, then return. The keeper ahead uses one full "
        "health spirit, combining this burst with a ranged attack."};
    return "LESSON " + std::to_string(std::min(s.lessons[i] + 1, lesson_count(site))) + " / " +
           std::to_string(lesson_count(site)) + "|" + tips[i] +
           "|Each lesson is one short duel, followed by free recovery. Return here for the next "
           "lesson. Your starter learns before the collection grows.";
}
int ground(int region, int x, int y) {
    if (region < 0 || region >= RegionCount || x < 0 || y < 0 || x >= MapWidth || y >= MapHeight)
        return 2;
    return Maps[region][y * MapWidth + x];
}
bool passable(int region, int x, int y) {
    int tile = ground(region, x, y);
    if (tile == 2 || tile == 3)
        return false;
    // Building footprints occupy their back tiles, leaving the door reachable.
    for (const auto &s : Regions[region].sites)
        if ((s.kind == Sanctuary || s.kind == Keeper) && std::abs(x - s.x) <= 1 && y == s.y - 1)
            return false;
    return true;
}
bool validate(const State &s) {
    if (s.walk_region < -1 || s.walk_region >= RegionCount || s.walk_depth < 0 ||
        s.walk_depth >= 8 || s.walk_choice < 0 || s.walk_choice > 1 ||
        (s.walk_region >= 0 && (s.walk_region != s.region || !available(s, s.region, 18))) ||
        (s.walk_region < 0 && s.walk_depth != 0))
        return false;
    for (int n : s.walks)
        if (n < 0 || n > 9999)
            return false;
    if (!s.seed || !accessible(s, s.region) || s.x < 256 || s.x >= (MapWidth - 1) * 256 ||
        s.y < 256 || s.y >= (MapHeight - 1) * 256 || !passable(s.region, s.x / 256, s.y / 256) ||
        s.threads < 0 || s.threads > 9999 || s.herbs < 0 || s.herbs > 99 || s.lead < 0 ||
        s.lead > 2 || s.ending < 0 || s.ending > 2 || (s.ending && !s.cleared[index(7, 16)]) ||
        s.defeats < 0 || s.defeats > 1000000 || s.victories < 0 || s.victories > 1000000 ||
        s.play_seconds < 0 || s.play_seconds > 100000000)
        return false;
    if (!valid_species(s.party[s.lead]))
        return false;
    for (int j = 0; j < 3; ++j) {
        int id = s.party[j];
        if (id < -1 || id >= 40 || (id >= 0 && !befriended(s, id)))
            return false;
        for (int k = 0; k < j; ++k)
            if (id >= 0 && id == s.party[k])
                return false;
    }
    for (int i = 0; i < SpeciesCount; ++i) {
        const auto &c = s.companions[i];
        if (c.trust < 0 || c.trust > trust_needed(i) || c.experience < 0 || c.experience > 99999 ||
            c.vitality < 0 || c.vitality > 1000 || c.temperament < 0 || c.temperament >= 5 ||
            c.charm < 0 || c.charm > 5 || (!charm_unlocked(s, i, c.charm) && c.charm != 0) ||
            !c.seed)
            return false;
    }
    for (int r = 0; r < RegionCount; ++r) {
        if (s.visits[r] < 0 || s.visits[r] > 1 || (s.visits[r] && !accessible(s, r)))
            return false;
        for (int n = 0; n < SiteCount; ++n) {
            int done = s.cleared[index(r, n)];
            if (done < 0 || done > 1 || (done && !available(s, r, n)))
                return false;
        }
    }
    for (int j = 0; j < 4; ++j)
        if (s.lessons[j] < 0 || s.lessons[j] > (j < 2 ? 4 : 3))
            return false;
    return true;
}
std::vector<uint8_t> serialize(const State &state) {
    std::vector<uint8_t> out{'T', 'I', 'N', 'I', 'S', 'A', 'V', '3'};
    auto put = [&](auto v) {
        for (int j = 0; j < 4; ++j)
            out.push_back(uint8_t(uint32_t(v) >> (j * 8)));
    };
    State copy = state;
    fields(copy, put);
    put(checksum(out.data(), out.size()));
    return out;
}
bool deserialize(State &s, const uint8_t *p, size_t n) {
    bool legacy = p && n == serialize(State{}).size() - 16 && std::equal(p, p + 8, "TINISAV2");
    if (!legacy && (!p || n != serialize(State{}).size() || !std::equal(p, p + 8, "TINISAV3")))
        return false;
    auto word = [&](size_t a) {
        return uint32_t(p[a]) | uint32_t(p[a + 1]) << 8 | uint32_t(p[a + 2]) << 16 |
               uint32_t(p[a + 3]) << 24;
    };
    if (word(n - 4) != checksum(p, n - 4))
        return false;
    State candidate;
    size_t at = 8;
    fields(
        candidate,
        [&](auto &v) {
            v = decltype(v + 0)(word(at));
            at += 4;
        },
        legacy);
    if (legacy) {
        int sites[] = {3, 6, 9, 12};
        int credited = 0;
        for (int j = 0; j < 4; ++j) {
            if (candidate.cleared[sites[j]])
                candidate.lessons[j] = lesson_count(sites[j]);
            credited += candidate.lessons[j];
        }
        for (int id : candidate.party)
            if (id >= 0 && id < SpeciesCount)
                candidate.companions[id].experience =
                    std::max(candidate.companions[id].experience, credited * 15);
    }
    if (!validate(candidate))
        return false;
    s = candidate;
    return true;
}
bool load(State &s, const std::filesystem::path &path, std::string &error) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        error = "No journey save at " + path.string();
        return false;
    }
    if (in.tellg() != std::streampos(serialize(State{}).size()) &&
        in.tellg() != std::streampos(serialize(State{}).size() - 16)) {
        error = "Journey save has the wrong size";
        return false;
    }
    std::vector<uint8_t> bytes(size_t(in.tellg()));
    in.seekg(0);
    if (!in.read(reinterpret_cast<char *>(bytes.data()), bytes.size()) ||
        !deserialize(s, bytes.data(), bytes.size())) {
        error = "Journey save is corrupt or incompatible";
        return false;
    }
    error.clear();
    return true;
}
bool save(const State &s, const std::filesystem::path &path, std::string &error) {
    if (!validate(s)) {
        error = "Refused invalid journey state";
        return false;
    }
    std::error_code ec;
    if (!path.parent_path().empty())
        std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    auto bytes = serialize(s);
    auto temp = path;
    temp += ".tmp";
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size()) ||
            !out.flush()) {
            error = "Could not write journey save";
            return false;
        }
    }
    // Preserve a validated last-good save before replacing it. A corrupt source never replaces
    // backup.
    State old;
    std::string ignored;
    if (load(old, path, ignored)) {
        auto backup = path;
        backup += ".bak";
        std::filesystem::copy_file(path, backup, std::filesystem::copy_options::overwrite_existing,
                                   ec);
        if (ec) {
            error = "Could not preserve backup: " + ec.message();
            return false;
        }
    }
#ifdef _WIN32
    if (!MoveFileExW(temp.c_str(), path.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        ec = std::error_code(int(GetLastError()), std::system_category());
#else
    std::filesystem::rename(temp, path, ec);
#endif
    if (ec) {
        error = ec.message();
        return false;
    }
    error.clear();
    return true;
}
} // namespace creature::campaign

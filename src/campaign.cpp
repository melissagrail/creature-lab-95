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
    if (starter != 0 && starter != 6 && starter != 35)
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
    if (Regions[region].sites[site].kind == Wild &&
        s.companions[Regions[region].sites[site].species].trust > 0)
        return true;
    int prior = Regions[region].sites[site].prerequisite;
    return prior < 0 || s.cleared[index(region, prior)];
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
    if (!valid_species(species) || befriended(s, species) || s.threads < 3 ||
        s.companions[species].trust < 1)
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
        static const int practice[] = {6, 35, 6, 35};
        e.enemies[0] = site <= 6 ? (step % 2 ? 6 : 35) : practice[(step + lesson_index(site)) % 4];
        e.styles[0] = step % 2;
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
    if (e.region == 0 && !e.walk && e.site >= 0 && e.site < SiteCount &&
        Regions[0].sites[e.site].kind == Trial) {
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
        if (n.kind == Wild) {
            auto &c = s.companions[n.species];
            c.trust = std::min(trust_needed(n.species), c.trust + 1);
            fill_party(s, n.species);
        }
        if (n.kind == Keeper)
            rest(s);
    } else {
        // A first completed friendly challenge builds recognition even in defeat.
        // Retreat is not a completed meeting; it never grants acquisition progress.
        if (!withdrew && n.kind == Wild && s.companions[n.species].trust == 0) {
            s.companions[n.species].trust = 1;
            fill_party(s, n.species);
        }
        ++s.defeats;
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
    configure_development(w, 0, own_growth.arts, own_growth.pace, own_growth.capacity,
                          own_growth.recovery);
    configure_development(w, 1, foe_growth.arts, foe_growth.pace, foe_growth.capacity,
                          foe_growth.recovery);
    if (e.region == 0 && !e.walk) {
        // Teach the basic attack/recovery loop before wind, surfaces and territory contests.
        w.surfaces = {};
        w.wind_base = {};
        w.winds = {};
        w.rain = w.wetness = 0;
        w.vane_enabled = 0;
        if (e.site <= 6) {
            w.objective = 0;
            w.obstacles = {};
            w.bodies[0].pos = {9 * Q, 9 * Q};
            w.bodies[1].pos = {15 * Q, 9 * Q};
        }
        if (Regions[0].sites[e.site].kind == Trial) {
            w.obstacles = {};
            w.bodies[0].pos = {9 * Q, 9 * Q};
            w.bodies[1].pos = {15 * Q, 9 * Q};
            w.bodies[1].hp = std::max(1, w.bodies[1].hp * (e.site == 3 ? 45 : 60) / 100);
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
        "ONE ART, ONE OPPONENT. Your teacher attacks, then pauses to let you answer. Face your "
        "partner, plant your feet to attack, then let your energy return. Watch the bright windup "
        "before stepping clear.",
        "FIND YOUR RHYTHM. Approach facing forward. Backing away is slower. Pause between attacks "
        "to recover breath. Eight successful lessons open the first wild habitat.",
        "A SECOND CHOICE. Your teacher leaves the lotus for you to claim. Keepers will contest it "
        "later. At bond 2, your second art and a paid dodge awaken. They share energy. The lotus "
        "in the centre now offers another way to win.",
        "PUT IT TOGETHER. Choose when to attack, dodge, or hold the centre. Your first companion "
        "can join you. The keeper ahead is your first three-opponent relay."};
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
        int credited=0;
        for (int j = 0; j < 4; ++j) {
            if (candidate.cleared[sites[j]]) candidate.lessons[j] = lesson_count(sites[j]);
            credited+=candidate.lessons[j];
        }
        for(int id:candidate.party) if(id>=0 && id<SpeciesCount)
            candidate.companions[id].experience=std::max(candidate.companions[id].experience,credited*15);
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

// Real engine/controller playthrough: no fabricated battle outcomes or extra stats.
#include "creature/brain.hpp"
#include "creature/campaign.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
using namespace creature;
namespace c = creature::campaign;
struct Stats {
    int matches = 0, rounds = 0, ticks = 0, losses = 0, remedies = 0;
};
bool fight(c::State &s, const c::Encounter &e, Brain &brain, Stats &stats) {
    std::array<int, 3> health{};
    for (int j = 0; j < 3; ++j)
        health[j] = s.party[j] < 0 ? 0 : s.companions[s.party[j]].vitality;
    int own = s.lead, round = 0, foe = 1000;
    bool success = false;
    ++stats.matches;
    while (round < e.rounds) {
        if (health[own] <= 0) {
            own = -1;
            for (int j = 0; j < 3; ++j)
                if (health[j] > 0) {
                    own = j;
                    break;
                }
            if (own < 0)
                break;
        }
        World w;
        c::initialize_round(w, s, e, round, own, health[own], foe);
        BrainMemory memory{}, enemy_memory{};
        bool remedy = false;
        auto &comp = s.companions[s.party[own]];
        auto personality = personality_preset(comp.temperament),
             variation = personality_from_seed(comp.seed);
        for (int i = 0; i < 3; ++i)
            personality[i] = std::clamp(personality[i] + variation[i] * .15f, -1.f, 1.f);
        while (!w.terminal && !w.truncated) {
            if (!remedy && s.herbs > 0 && w.bodies[0].hp < Roster[s.party[own]].hp * .55) {
                w.bodies[0].hp =
                    std::min(Roster[s.party[own]].hp, w.bodies[0].hp + Roster[s.party[own]].hp / 3);
                --s.herbs;
                remedy = true;
                ++stats.remedies;
            }
            auto opponent = c::trained_opponent(s, e)
                                ? brain.action(observe(w, 1), enemy_memory,
                                               personality_preset((e.region + round) % 5))
                                : c::opponent_action(w, e, round);
            auto result = step(
                w, {c::companion_action(w, c::beginner_assistance(w.bodies[0])
                                               ? Action{}
                                               : brain.action(observe(w, 0), memory, personality), comp.temperament),
                    opponent});
            stats.ticks += result.ticks;
            if (w.overflow) {
                std::cerr << "Engine capacity overflow\n";
                std::exit(2);
            }
        }
        ++stats.rounds;
        if (w.winner == 0) {
            health[own] = std::clamp(w.bodies[0].hp * 1000 / Roster[s.party[own]].hp, 1, 1000);
            ++round;
            foe = 1000;
            if (round == e.rounds) {
                success = true;
                break;
            }
            health[own] = std::min(1000, health[own] + (comp.charm == 3 ? 300 : 150));
        } else {
            health[own] = 0;
            foe = std::max(1, w.bodies[1].hp * 1000 / Roster[w.bodies[1].species].hp);
            int next = -1;
            for (int j = 1; j <= 3; ++j)
                if (health[(own + j) % 3] > 0) {
                    next = (own + j) % 3;
                    break;
                }
            if (next < 0)
                break;
            own = next;
        }
    }
    if (!c::resolve(s, e, success, health) || !c::validate(s)) {
        std::cerr << "Invalid campaign resolution\n";
        std::exit(2);
    }
    stats.losses += !success;
    return success;
}
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "campaign_playthrough MODEL OUT.csv [seed=20260907] [walks|restless]\n";
        return 1;
    }
    Brain brain;
    std::string error;
    if (!brain.load(argv[1], error)) {
        std::cerr << error << '\n';
        return 1;
    }
    std::ofstream out(argv[2]);
    out << "starter,seed,region,friends,battles,rounds,combat_seconds,losses,remedies,bells\n";
    uint32_t seed = argc > 3 ? uint32_t(std::stoul(argv[3])) : 20260907;
    bool blocked = false;
    bool restless = argc > 4 && std::string(argv[4]) == "restless";
    for (int starter : {0, 1, 16, 6, 35}) {
        if (restless && starter != 0)
            continue;
        auto s = c::new_journey(seed + starter, starter);
        Stats stats;
        for (int region = 0; region < 8; ++region) {
            if (region && !c::travel(s, region)) {
                blocked = true;
                break;
            }
            // Visible habitats first; a thread offering follows first contact.
            for (int site : {2, 5, 10, 7, 13}) {
                int species = c::Regions[region].sites[site].species;
                if (c::befriended(s, species) || !c::available(s, region, site))
                    continue;
                bool met = false;
                for (int attempt = 0; attempt < 12 && !met; ++attempt) {
                    c::rest(s);
                    fight(s, c::encounter(s, site), brain, stats);
                    met = s.companions[species].trust > 0;
                }
                if (!met) {
                    std::cerr << "Could not meet " << Roster[species].name
                              << " in twelve attempts, starter " << starter << '\n';
                    blocked = true;
                    continue;
                }
                while (!c::befriended(s, species) && c::offer_thread(s, species)) {
                }
                // Rotate the third slot to sample regional species while keeping two familiar
                // leads.
                if (c::befriended(s, species) && s.party[2] >= 0)
                    c::set_party(s, 2, species);
            }
            for (int site : {1, 3, 4, 6, 9, 11, 12, 16}) {
                if (c::Regions[region].sites[site].kind == c::Conversation) {
                    c::finish_story(s, site);
                    continue;
                }
                bool won = false;
                for (int attempt = 0; attempt < 24 && !won; ++attempt) {
                    c::rest(s);
                    if (attempt > 0) {
                        std::vector<int> owned;
                        for (int id = 0; id < 40; ++id)
                            if (c::befriended(s, id))
                                owned.push_back(id);
                        for (int slot = 0; slot < std::min(3, int(owned.size())); ++slot)
                            c::set_party(s, slot,
                                         owned[(attempt * 3 + site + slot) % owned.size()]);
                    }
                    s.lead = attempt % 3;
                    if (s.party[s.lead] < 0)
                        s.lead = 0;
                    if (attempt >= 2)
                        for (int id : s.party)
                            if (id >= 0) {
                                s.companions[id].temperament = attempt % 2;
                                if (c::charm_unlocked(s, id, 1))
                                    s.companions[id].charm = 1;
                            }
                    won = fight(s, c::encounter(s, site), brain, stats) &&
                          s.cleared[region * 20 + site];
                }
                if (!won) {
                    std::cerr << "Road blocked region " << region << " site " << site << " starter "
                              << starter << '\n';
                    blocked = true;
                    break;
                }
            }
            if (region == 0) {
                // Visible habitats first; a thread offering follows first contact.
                for (int site : {2, 5, 10, 7, 13}) {
                    int species = c::Regions[region].sites[site].species;
                    if (c::befriended(s, species) || !c::available(s, region, site))
                        continue;
                    bool met = false;
                    for (int attempt = 0; attempt < 12 && !met; ++attempt) {
                        c::rest(s);
                        fight(s, c::encounter(s, site), brain, stats);
                        met = s.companions[species].trust > 0;
                    }
                    if (!met) {
                        std::cerr << "Could not meet " << Roster[species].name
                                  << " in twelve attempts, starter " << starter << '\n';
                        blocked = true;
                        continue;
                    }
                    while (!c::befriended(s, species) && c::offer_thread(s, species)) {
                    }
                    // Rotate the third slot to sample regional species while keeping two familiar
                    // leads.
                    if (c::befriended(s, species) && s.party[2] >= 0)
                        c::set_party(s, 2, species);
                }
            }
            c::finish_story(s, 19);
            c::solve_puzzle(s, 15, c::puzzle_solution(region));
            c::finish_story(s, 8);
            if (region == 7 && c::restored_count(s) == 8)
                for (int id = 0; id < 40; ++id)
                    while (!c::befriended(s, id) && c::offer_thread(s, id)) {
                    }
            out << starter << ',' << s.seed << ',' << region << ',' << c::collection_count(s) << ','
                << stats.matches << ',' << stats.rounds << ',' << stats.ticks / 30. << ','
                << stats.losses << ',' << stats.remedies << ',' << c::restored_count(s) << '\n';
            out.flush();
            auto bytes = c::serialize(s);
            c::State copy;
            if (!c::deserialize(copy, bytes.data(), bytes.size()) || c::serialize(copy) != bytes)
                return 3;
        }
        if (c::restored_count(s) == 8) {
            for (int id = 0; id < 40; ++id)
                while (!c::befriended(s, id) && c::offer_thread(s, id)) {
                }
            s.ending = 1;
            if (!c::validate(s))
                return 4;
            s.ending = 2;
            if (!c::validate(s))
                return 4;
        }
        std::cout << "Starter " << Roster[starter].name << ": " << c::restored_count(s)
                  << " bells, " << c::collection_count(s) << " friends, " << stats.matches
                  << " encounters / " << stats.losses << " losses / " << stats.ticks / 1800.
                  << " combat minutes\n"
                  << std::flush;
        if (c::restored_count(s) == 8 && c::collection_count(s) != 40)
            blocked = true;
        if (starter == 0 && c::restored_count(s) == 8 && argc > 4 &&
            (std::string(argv[4]) == "walks" || restless)) {
            std::ofstream walks(std::string(argv[2]) + ".walks.csv");
            walks << "region,attempts,completed,encounters,losses,combat_seconds,remedies\n";
            for (int region = 0; region < 8; ++region) {
                c::travel(s, region);
                int before = s.walks[region], attempts = 0;
                Stats ws;
                while (s.walks[region] == before && attempts < 6) {
                    c::rest(s);
                    for (int slot = 0; slot < 3; ++slot)
                        c::set_party(s, slot, (region * 5 + attempts * 7 + slot * 11) % 40);
                    for (int id : s.party)
                        if (id >= 0) {
                            s.companions[id].temperament = attempts % 2;
                            if (c::charm_unlocked(s, id, 1))
                                s.companions[id].charm = 1;
                        }
                    s.lead = 0;
                    if (!c::begin_walk(s))
                        return 5;
                    ++attempts;
                    while (s.walk_region >= 0) {
                        if ((s.walk_depth == 2 || s.walk_depth == 5) && c::rest_walk(s))
                            continue;
                        if (restless)
                            for (int slot = 0; slot < 3; ++slot) {
                                if (s.companions[s.party[slot]].vitality >= 650)
                                    continue;
                                int fresh = -1;
                                for (int j = 0; j < 40; ++j) {
                                    int id = (region * 5 + s.walk_depth * 7 + j) % 40;
                                    if (std::find(s.party.begin(), s.party.end(), id) !=
                                        s.party.end())
                                        continue;
                                    if (c::befriended(s, id) &&
                                        (fresh < 0 ||
                                         s.companions[id].vitality > s.companions[fresh].vitality))
                                        fresh = id;
                                }
                                if (fresh >= 0) {
                                    c::set_party(s, slot, fresh);
                                    s.companions[fresh].temperament = attempts % 2;
                                    if (c::charm_unlocked(s, fresh, 1))
                                        s.companions[fresh].charm = 1;
                                }
                            }
                        s.walk_choice = restless ? 1 : 0;
                        fight(s, c::walk_encounter(s, s.walk_choice), brain, ws);
                    }
                }
                bool completed = s.walks[region] > before;
                walks << region << ',' << attempts << ',' << completed << ',' << ws.matches << ','
                      << ws.losses << ',' << ws.ticks / 30. << ',' << ws.remedies << '\n';
                walks.flush();
                std::cout << (restless ? "Restless" : "Quiet") << " Lantern Walk " << region << ": "
                          << completed << " in " << attempts << " attempts, " << ws.ticks / 1800.
                          << " combat minutes\n"
                          << std::flush;
                blocked |= !completed;
            }
        }
    }
    return blocked ? 2 : 0;
}

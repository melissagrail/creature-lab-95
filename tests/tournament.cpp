#include "creature/sim.hpp"
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
using namespace creature;
int main(int argc, char **argv) {
    int seeds = argc > 1 ? std::stoi(argv[1]) : 2;
    std::string path = argc > 2 ? argv[2] : "reports/matches.csv";
    int seed_offset = argc > 3 ? std::stoi(argv[3]) : 1000;
    std::ofstream out(path);
    if (!out || seeds < 1 || seeds > 128)
        return 1;
    out << "a,b,seed,weather,arena,style_a,style_b,winner,ticks,reason,hp_a,hp_b,control_a,control_"
           "b,overflow\n";
    long matches = 0, overflows = 0;
    std::array<double, 40> wins{};
    std::array<int, 40> games{};
    auto start = std::chrono::steady_clock::now();
    for (int a = 0; a < 40; a++)
        for (int b = a + 1; b < 40; b++)
            for (int seed = 0; seed < seeds; seed++)
                for (int seat = 0; seat < 2; seat++) {
                    int left = seat ? b : a, right = seat ? a : b, weather = seed % 3,
                        arena = (seed / 3) % ArenaCount, sa = (seed / (3 * ArenaCount)) % 4,
                        sb = (sa + 1) % 4;
                    World w;
                    reset(w, seed_offset + seed, weather, left, right, arena);
                    for (int t = 0; t < 900 && !w.terminal && !w.truncated; t++)
                        step(w, {scripted(w, 0, seat ? sb : sa), scripted(w, 1, seat ? sa : sb)});
                    out << left << ',' << right << ',' << seed_offset + seed << ',' << weather
                        << ',' << arena << ',' << (seat ? sb : sa) << ',' << (seat ? sa : sb) << ','
                        << w.winner << ',' << w.tick << ',' << w.end_reason << ',' << w.bodies[0].hp
                        << ',' << w.bodies[1].hp << ',' << w.bodies[0].control << ','
                        << w.bodies[1].control << ',' << w.overflow << '\n';
                    games[left]++;
                    games[right]++;
                    if (w.winner < 0) {
                        wins[left] += .5;
                        wins[right] += .5;
                    } else
                        wins[w.winner == 0 ? left : right]++;
                    matches++;
                    overflows += w.overflow;
                }
    std::ofstream meta(path + ".meta.json");
    meta << "{\"rules_version\":" << RulesVersion
         << ",\"observation_version\":" << ObservationVersion << ",\"content_hash\":" << ContentHash
         << ",\"seed_offset\":" << seed_offset << ",\"seeds\":" << seeds
         << ",\"arena_count\":" << ArenaCount << ",\"matches\":" << matches
         << ",\"pilot\":\"four scripted styles; no trained models\"}\n";
    double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    for (int i = 0; i < 40; i++)
        std::cout << i << ' ' << Roster[i].name << ' ' << int(wins[i] * 1000 / games[i]) / 10.0
                  << "%\n";
    std::cout << matches << " matches in " << sec << "s; overflow " << overflows << '\n';
    return overflows ? 2 : 0;
}

#include "creature/sim.hpp"
#include <iostream>
using namespace creature;
int main() {
    int matches = 0;
    int64_t ticks = 0, casts = 0, samples = 0, low = 0, energy = 0;
    for (int sp = 0; sp < SpeciesCount; ++sp)
        for (int scenario = 0; scenario < 9; ++scenario)
            for (int seat = 0; seat < 2; ++seat) {
                World w;
                int other = (sp + 13) % SpeciesCount;
                reset(w, 4200 + scenario, scenario % 3, seat ? other : sp, seat ? sp : other,
                      scenario / 3);
                while (!w.terminal && !w.truncated) {
                    auto result =
                        step(w, {scripted(w, 0, seat ? 1 : 0), scripted(w, 1, seat ? 0 : 1)});
                    for (int i = 0; i < 2; ++i) {
                        casts += result.features[i].spent > 0;
                        low += w.bodies[i].energy < Moves[160].cost;
                        energy += w.bodies[i].energy;
                        ++samples;
                    }
                }
                ticks += w.tick;
                ++matches;
            }
    std::cout << "{\"rules\":" << RulesVersion << ",\"content_hash\":" << ContentHash
              << ",\"matches\":" << matches
              << ",\"mean_duration_seconds\":" << double(ticks) / Hz / matches
              << ",\"casts_per_spirit_minute\":" << double(casts) * Hz * 60 / (2 * ticks)
              << ",\"below_dodge_cost_percent\":" << 100.0 * low / samples
              << ",\"mean_displayed_energy\":" << double(energy) / samples / 10 << "}\n";
}

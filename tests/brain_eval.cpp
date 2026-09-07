#include "creature/brain.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace creature;
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "brain_eval MODEL|scripted OUT.csv [scenarios=240] [seed=2100000000] "
                     "[personality=steady|all] [opponent=scripted|MODEL] [grid=legacy|crossed]\n";
        return 1;
    }
    int scenarios = argc > 3 ? std::stoi(argv[3]) : 240;
    uint32_t base = argc > 4 ? uint32_t(std::stoul(argv[4])) : 2100000000u;
    if (scenarios < 1 || scenarios > 10000)
        return 1;
    std::string preset = argc > 5 ? argv[5] : "steady",
                opponent_path = argc > 6 ? argv[6] : "scripted";
    std::string grid = argc > 7 ? argv[7] : "legacy";
    if (grid != "legacy" && grid != "crossed") {
        std::cerr << "Unknown evaluation grid\n";
        return 1;
    }
    std::vector<int> profiles;
    for (int p = 0; p < PersonalityCount; ++p) {
        std::string name = personality_name(p);
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return char(std::tolower(c)); });
        if (preset == "all" || preset == name)
            profiles.push_back(p);
    }
    if (profiles.empty()) {
        std::cerr << "Unknown personality\n";
        return 1;
    }
    Brain brain, opponent_brain;
    std::string error;
    bool teacher = std::string(argv[1]) == "scripted",
         learned_opponent = opponent_path != "scripted";
    if (!teacher && !brain.load(argv[1], error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (learned_opponent && !opponent_brain.load(opponent_path, error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (!teacher && brain.format() == 2 && (profiles.size() > 1 || profiles[0] != 0)) {
        std::cerr << "Legacy models support steady only\n";
        return 1;
    }
    std::ofstream out(argv[2]);
    if (!out)
        return 1;
    out << "case,species,opponent,arena,weather,seed,seat,style,score,ticks,reason,own_hp,enemy_hp,"
           "casts,energy_spent,overflow,personality,decisions,distance,objective_distance,mean_"
           "energy,"
           "stationary_fraction,retreat_fraction,low_hp_decisions,low_hp_retreats,casts_1,casts_2,"
           "casts_3,casts_4,casts_5,damage,damage_taken,healed,shielded,control,model_checksum,"
           "opponent_checksum\n";
    int total_overflow = 0;
    for (int profile : profiles) {
        double score = 0, ticks = 0;
        for (int c = 0; c < scenarios; ++c)
            for (int seat = 0; seat < 2; ++seat) {
                int species = c % 40, opponent = (species + 17 + 7 * (c / 40)) % 40,
                    arena = (c / 40 + c) % 6,
                    weather = grid == "crossed" ? (c / 240) % 3 : (c / 40 + c) % 3,
                    style = (c / 40 + c) % 4;
                World w;
                reset(w, base + c, weather, seat ? opponent : species, seat ? species : opponent,
                      arena);
                BrainMemory memory{}, opponent_memory{};
                int casts = 0, spent = 0, n = 0, stationary = 0, retreat = 0, low_hp = 0,
                    low_retreat = 0;
                std::array<int, 5> slots{};
                int damage = 0, taken = 0, healed = 0, shielded = 0;
                double distance = 0, center = 0, energy = 0;
                while (!w.terminal && !w.truncated) {
                    auto before = w.bodies[seat].pos, enemy = w.bodies[1 - seat].pos;
                    bool fragile = w.bodies[seat].hp < Roster[species].hp * .4;
                    distance +=
                        std::hypot(double(enemy.x - before.x), double(enemy.y - before.y)) / Q;
                    center += std::hypot(double(before.x - 12 * Q), double(before.y - 9 * Q)) / Q;
                    energy += w.bodies[seat].energy / 1000.;
                    std::array<Action, 2> actions;
                    actions[seat] = teacher ? scripted(w, seat, 0)
                                            : brain.action(observe(w, seat), memory,
                                                           personality_preset(profile));
                    actions[1 - seat] =
                        learned_opponent
                            ? opponent_brain.action(observe(w, 1 - seat), opponent_memory)
                            : scripted(w, 1 - seat, style);
                    int tick_before = w.tick;
                    auto result = step(w, actions);
                    const auto &f = result.features[seat];
                    spent += f.spent;
                    // Footwork also spends energy. Count accepted cast events, not costs.
                    for (const auto &event : w.events) {
                        if (event.tick < tick_before || event.tick >= w.tick ||
                            event.kind != Started || event.actor != seat)
                            continue;
                        ++casts;
                        for (int slot = 0; slot < 5; ++slot)
                            if (move_id(w.bodies[seat], slot) == event.move)
                                ++slots[slot];
                    }
                    damage += f.dealt;
                    taken += f.taken;
                    healed += f.healed;
                    shielded += f.shielded;
                    auto after = w.bodies[seat].pos;
                    double dx = after.x - before.x, dy = after.y - before.y;
                    bool still = std::hypot(dx, dy) * 30 / std::max(1, result.ticks) / Q < .2;
                    bool back = !still && dx * (enemy.x - before.x) + dy * (enemy.y - before.y) < 0;
                    stationary += still;
                    retreat += back;
                    low_hp += fragile;
                    low_retreat += fragile && back;
                    ++n;
                }
                double point = w.winner < 0 ? .5 : double(w.winner == seat);
                score += point;
                ticks += w.tick;
                total_overflow += w.overflow;
                out << c << ',' << species << ',' << opponent << ',' << arena << ',' << weather
                    << ',' << base + c << ',' << seat << ',' << style << ',' << point << ','
                    << w.tick << ',' << w.end_reason << ',' << w.bodies[seat].hp << ','
                    << w.bodies[1 - seat].hp << ',' << casts << ',' << spent << ',' << w.overflow
                    << ',' << personality_name(profile) << ',' << n << ',' << distance / n << ','
                    << center / n << ',' << energy / n << ',' << double(stationary) / n << ','
                    << double(retreat) / n << ',' << low_hp << ',' << low_retreat;
                for (int count : slots)
                    out << ',' << count;
                out << ',' << damage << ',' << taken << ',' << healed << ',' << shielded << ','
                    << w.bodies[seat].control << ',' << std::hex << brain.checksum() << ','
                    << opponent_brain.checksum() << std::dec << '\n';
            }
        std::cout << personality_name(profile) << ": " << scenarios * 2 << " native matches; score "
                  << score / (scenarios * 2) << "; mean seconds " << ticks / (scenarios * 2 * 30)
                  << '\n'
                  << std::flush;
    }
    return total_overflow ? 2 : 0;
}

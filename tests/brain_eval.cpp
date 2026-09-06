#include "creature/brain.hpp"
#include <fstream>
#include <iostream>
#include <string>
using namespace creature;
int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "brain_eval MODEL|scripted OUT.csv [scenarios=240] [seed=1900000000]\n";
        return 1;
    }
    int scenarios = argc > 3 ? std::stoi(argv[3]) : 240;
    uint32_t base = argc > 4 ? uint32_t(std::stoul(argv[4])) : 1900000000u;
    if (scenarios < 1 || scenarios > 10000)
        return 1;
    Brain brain;
    std::string error;
    bool teacher = std::string(argv[1]) == "scripted";
    if (!teacher && !brain.load(argv[1], error)) {
        std::cerr << error << '\n';
        return 1;
    }
    std::ofstream out(argv[2]);
    if (!out)
        return 1;
    out << "case,species,opponent,arena,weather,seed,seat,style,score,ticks,reason,own_hp,enemy_hp,"
           "casts,energy_spent,overflow\n";
    double score = 0, ticks = 0;
    int overflow = 0;
    for (int c = 0; c < scenarios; ++c)
        for (int seat = 0; seat < 2; ++seat) {
            int species = c % 40, opponent = (species + 17 + 7 * (c / 40)) % 40,
                arena = (c / 40 + c) % 6, weather = (c / 40 + c) % 3, style = (c / 40 + c) % 4;
            World w;
            reset(w, base + c, weather, seat ? opponent : species, seat ? species : opponent,
                  arena);
            BrainMemory memory{};
            int casts = 0, spent = 0;
            while (!w.terminal && !w.truncated) {
                std::array<Action, 2> actions;
                actions[seat] =
                    teacher ? scripted(w, seat, 0) : brain.action(observe(w, seat), memory);
                actions[1 - seat] = scripted(w, 1 - seat, style);
                auto result = step(w, actions);
                spent += result.features[seat].spent;
                casts += result.features[seat].spent > 0;
            }
            double point = w.winner < 0 ? .5 : double(w.winner == seat);
            score += point;
            ticks += w.tick;
            overflow += w.overflow;
            out << c << ',' << species << ',' << opponent << ',' << arena << ',' << weather << ','
                << base + c << ',' << seat << ',' << style << ',' << point << ',' << w.tick << ','
                << w.end_reason << ',' << w.bodies[seat].hp << ',' << w.bodies[1 - seat].hp << ','
                << casts << ',' << spent << ',' << w.overflow << '\n';
        }
    std::cout << scenarios * 2 << " native matches; score " << score / (scenarios * 2)
              << "; mean seconds " << ticks / (scenarios * 2 * 30) << "; overflow " << overflow
              << '\n';
    return overflow ? 2 : 0;
}

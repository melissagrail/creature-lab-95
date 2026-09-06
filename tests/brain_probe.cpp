#include "creature/brain.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
using namespace creature;
int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "brain_probe MODEL OUT.csv\n";
        return 1;
    }
    Brain brain;
    std::string error;
    if (!brain.load(argv[1], error) || brain.format() != 3) {
        std::cerr << "Format-3 brain required: " << error << '\n';
        return 1;
    }
    std::ofstream out(argv[2]);
    if (!out)
        return 1;
    out << "species,personality,decisions,changed_from_steady,approach_request,cast_requests,"
           "selectable_decisions,requested_cost,slot_1,slot_2,slot_3,slot_4,slot_5,model_checksum\n";
    for (int species = 0; species < 40; ++species) {
        std::array<int, 5> count{}, changed{}, casts{}, selectable{}, cost{};
        std::array<double, 5> approach{};
        std::array<std::array<int, 5>, 5> slots{};
        for (int seat = 0; seat < 2; ++seat) {
            World w;
            int enemy = (species + 17) % 40;
            reset(w, 910000000u + species * 2 + seat, species % 3, seat ? enemy : species,
                  seat ? species : enemy, species % 6);
            std::array<BrainMemory, 5> memory{};
            for (int tick = 0; tick < 300 && !w.terminal && !w.truncated; ++tick) {
                auto obs = observe(w, seat);
                auto before = hash(w);
                std::array<Action, 5> actions{};
                double dx = w.bodies[1 - seat].pos.x - w.bodies[seat].pos.x,
                       dy = w.bodies[1 - seat].pos.y - w.bodies[seat].pos.y;
                double length = std::max(1., std::hypot(dx, dy));
                for (int p = 0; p < 5; ++p) {
                    auto a = brain.action(obs, memory[p], personality_preset(p));
                    actions[p] = a;
                    ++count[p];
                    auto n = actions[0];
                    changed[p] += a.mx != n.mx || a.my != n.my || a.ax != n.ax || a.ay != n.ay ||
                                  a.ability != n.ability;
                    approach[p] += (a.mx * dx + a.my * dy) / length / Q;
                    bool available = false;
                    for (int j = 1; j < 6; ++j)
                        available |= obs.mask[j] > .5f;
                    selectable[p] += available;
                    if (a.ability > 0) {
                        ++casts[p];
                        ++slots[p][a.ability - 1];
                        cost[p] += move_for(w.bodies[seat], a.ability - 1).cost;
                    }
                }
                if (hash(w) != before) {
                    std::cerr << "Controller mutated observation source\n";
                    return 2;
                }
                step(w, {scripted(w, 0), scripted(w, 1)});
            }
        }
        for (int p = 0; p < 5; ++p) {
            out << species << ',' << personality_name(p) << ',' << count[p] << ',' << changed[p]
                << ',' << approach[p] / count[p] << ',' << casts[p] << ',' << selectable[p] << ','
                << cost[p];
            for (int c : slots[p])
                out << ',' << c;
            out << ',' << std::hex << brain.checksum() << std::dec << '\n';
        }
    }
    std::cout << "40 species / five temperaments / identical scripted observation traces; no world "
                 "mutation\n";
}

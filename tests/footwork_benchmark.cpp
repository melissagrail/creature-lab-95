#include "creature/sim.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
using namespace creature;
int main(int argc, char **argv) {
    std::ofstream out(argc > 1 ? argv[1] : "reports/footwork.csv");
    out << "species,mode,energy_start,energy_end,mean_speed,mean_regen,damage_dealt,damage_taken,"
           "control,ticks\n";
    for (int sp = 0; sp < 40; sp++)
        for (int mode = 0; mode < 4; mode++) {
            World w;
            reset(w, 1000 + sp, 0, sp, (sp + 17) % 40, 2);
            w.surfaces = {};
            w.obstacles = {};
            w.vane_enabled = 0;
            w.objective = 0;
            w.bodies[0].energy = 600;
            int speed = 0, regen = 0, dealt = 0, taken = 0, ticks = 0;
            for (int t = 0; t < 600 && !w.terminal && !w.truncated; t += 3) {
                Action a = scripted(w, 0);
                Vec d = w.bodies[1].pos - w.bodies[0].pos;
                Vec dir = unit(d);
                if (mode == 0 || mode == 1) {
                    Vec move = unit(Vec{-d.y, d.x});
                    a.mx = move.x;
                    a.my = move.y;
                    if (mode == 1) {
                        a.ax = move.x;
                        a.ay = move.y;
                    } else {
                        a.ax = dir.x;
                        a.ay = dir.y;
                    }
                    a.ability = 0;
                }
                if (mode == 2) {
                    a.mx = a.my = 0;
                    a.ability = 0;
                }
                auto result = step(w, {a, scripted(w, 1)});
                ticks += result.ticks;
                dealt += result.features[0].dealt;
                taken += result.features[0].taken;
                speed += length(w.bodies[0].vel);
                regen += energy_regen(w.bodies[0]);
            }
            int samples = std::max(1, ticks / 3);
            out << sp << ','
                << (mode == 0   ? "combat_strafe"
                    : mode == 1 ? "face_travel"
                    : mode == 2 ? "planted"
                                : "scripted")
                << ",600," << w.bodies[0].energy << ',' << double(speed) / samples << ','
                << double(regen) / samples << ',' << dealt << ',' << taken << ','
                << w.bodies[0].control << ',' << ticks << '\n';
        }
    std::cout << "Footwork scenarios recorded\n";
}

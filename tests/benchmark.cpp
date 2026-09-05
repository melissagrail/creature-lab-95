#include "creature/sim.hpp"
#include <chrono>
#include <iostream>
#include <vector>
using namespace creature;
int main() {
    std::vector<World> worlds(256);
    for (int i = 0; i < 256; i++)
        reset(worlds[i], i + 1, i % 3);
    long ticks = 0;
    auto start = std::chrono::steady_clock::now();
    for (int k = 0; k < 1000; k++)
        for (auto &w : worlds) {
            if (w.terminal || w.truncated)
                reset(w, w.rng, 1);
            ticks += step(w, {scripted(w, 0), scripted(w, 1)}).ticks;
        }
    double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "256 arenas, single thread, scripted inference included\n"
              << ticks << " physics ticks / " << sec << " seconds = " << long(ticks / sec)
              << " ticks/s; " << long(256000 / sec) << " joint decisions/s\n"
              << "World bytes: " << sizeof(World)
              << "; snapshot bytes: " << snapshot(worlds[0]).size() << "\n";
}

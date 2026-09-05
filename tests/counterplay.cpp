#include "creature/sim.hpp"
#include <array>
#include <fstream>
#include <iostream>
using namespace creature;
int main() {
    const std::array<std::array<int, 2>, 10> pairs{{{4, 10},
                                                    {4, 35},
                                                    {4, 12},
                                                    {1, 8},
                                                    {3, 24},
                                                    {24, 38},
                                                    {22, 25},
                                                    {5, 12},
                                                    {17, 25},
                                                    {24, 34}}};
    std::ofstream out("reports/counterplay.csv");
    if (!out)
        return 1;
    out << "species_a,species_b,style_a,style_b,weather,arena,seed,seat,score_a\n";
    for (auto pair : pairs) {
        double scores[4] = {};
        int games[4] = {};
        for (int a = 0; a < 4; a++)
            for (int b = 0; b < 4; b++)
                for (int weather = 0; weather < 3; weather++)
                    for (int arena = 0; arena < 3; arena++)
                        for (int seed = 0; seed < 2; seed++)
                            for (int seat = 0; seat < 2; seat++) {
                                World w;
                                reset(w, 4000 + seed, weather, pair[seat], pair[1 - seat], arena);
                                while (!w.terminal && !w.truncated)
                                    step(w, {scripted(w, 0, seat ? b : a),
                                             scripted(w, 1, seat ? a : b)});
                                double score = w.winner < 0 ? .5 : w.winner == seat ? 1. : 0.;
                                scores[a] += score;
                                games[a]++;
                                out << pair[0] << ',' << pair[1] << ',' << a << ',' << b << ','
                                    << weather << ',' << arena << ',' << 4000 + seed << ',' << seat
                                    << ',' << score << '\n';
                            }
        std::cout << Roster[pair[0]].name << " vs " << Roster[pair[1]].name << ":";
        for (int a = 0; a < 4; a++)
            std::cout << ' ' << scores[a] / games[a];
        std::cout << '\n';
    }
}

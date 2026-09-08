#include "creature/brain.hpp"
#include "creature/campaign.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
using namespace creature;
namespace c = creature::campaign;
int main(int argc, char **argv) {
    if (argc < 3 || argc > 4)
        return 1;
    bool calls = argc == 4 && std::string(argv[3]) == "calls";
    Brain brain;
    std::string error;
    if (!brain.load(argv[1], error)) {
        std::cerr << error;
        return 1;
    }
    std::ofstream out(argv[2]);
    out << "starter,seed,site,lesson,won,seconds,casts,energy_spent,own_rank,foe_rank,locked_"
           "casts,reason,own_hp,foe_hp,own_control,foe_control,keeper_calls,damage_taken\n";
    int completed = 0, failures = 0, locked = 0, total = 0, wins = 0;
    double seconds = 0;
    for (int starter : {0, 1, 16, 6, 35})
        for (int sample = 0; sample < 24; ++sample) {
            auto state = c::new_journey(24000000u + sample * 100 + starter, starter);
            c::finish_story(state, 1);
            bool blocked = false;
            for (int site : {3, 6, 9, 12, 16}) {
                if (site == 6)
                    c::finish_story(state, 4);
                if (site == 12)
                    c::finish_story(state, 11);
                for (int attempt = 0; attempt < 16 && !state.cleared[site]; ++attempt) {
                    c::rest(state);
                    auto e = c::encounter(state, site);
                    World w;
                    c::initialize_round(w, state, e, 0, 0, 1000, 1000);
                    BrainMemory memory{};
                    auto &comp = state.companions[starter];
                    auto personality = personality_preset(comp.temperament),
                         variation = personality_from_seed(comp.seed);
                    for (int j = 0; j < 3; ++j)
                        personality[j] =
                            std::clamp(personality[j] + .15f * variation[j], -1.f, 1.f);
                    bool remedy = false;
                    int casts = 0, spent = 0, illegal = 0, called = 0, damage = 0;
                    int last_charge = -1, last_recovery = -1;
                    int rank = c::rank(comp);
                    int lesson = site == 16 ? 0 : state.lessons[c::lesson_index(site)] + 1;
                    while (!w.terminal && !w.truncated) {
                        if (w.bodies[0].guidance_age >= 90)
                            command(w, 0, Free);
                        const auto &foe = w.bodies[1];
                        if (calls && foe.move >= 0 && Moves[foe.move].kind == Nova) {
                            int cast_tick = w.tick - foe.age;
                            if (phase(foe) == Startup && foe.age >= 9 && last_charge != cast_tick) {
                                command(w, 0, Retreat);
                                last_charge = cast_tick;
                                ++called;
                            }
                            if (phase(foe) == Recovery && last_recovery != cast_tick) {
                                command(w, 0, Attack);
                                last_recovery = cast_tick;
                                ++called;
                            }
                        }
                        if (!remedy && state.herbs > 0 && w.bodies[0].hp < Roster[starter].hp / 2) {
                            --state.herbs;
                            remedy = true;
                            w.bodies[0].hp = std::min(Roster[starter].hp,
                                                      w.bodies[0].hp + Roster[starter].hp / 3);
                        }
                        auto a = c::companion_action(
                            w, c::beginner_assistance(w.bodies[0])
                                   ? Action{}
                                   : brain.action(observe(w, 0), memory, personality));
                        illegal += a.ability && !(w.bodies[0].arts & (1 << (a.ability - 1)));
                        int head = w.event_head;
                        auto result = step(w, {a, c::opponent_action(w, e, 0)});
                        spent += result.features[0].spent;
                        damage += result.features[0].taken;
                        for (int j = 0; j < HistoryCount; ++j) {
                            const auto &event = w.events[(head + j) % HistoryCount];
                            if (event.kind == Started && event.actor == 0 &&
                                event.tick >= w.tick - result.ticks && event.tick < w.tick)
                                ++casts;
                        }
                    }
                    bool won = w.winner == 0;
                    double duration = w.tick / double(Hz);
                    out << starter << ',' << state.seed << ',' << site << ',' << lesson << ','
                        << won << ',' << duration << ',' << casts << ',' << spent << ',' << rank
                        << ',' << (site < 9 ? 1 : 2) << ',' << illegal << ',' << w.end_reason << ','
                        << w.bodies[0].hp << ',' << w.bodies[1].hp << ',' << w.bodies[0].control
                        << ',' << w.bodies[1].control << ',' << called << ',' << damage << '\n';
                    ++total;
                    wins += won;
                    seconds += duration;
                    locked += illegal;
                    if (!c::resolve(state, e, won, {won ? 800 : 0, 0, 0}) || !c::validate(state))
                        return 2;
                }
                if (!state.cleared[site]) {
                    blocked = true;
                    break;
                }
            }
            completed += !blocked;
            failures += blocked;
        }
    std::cout << completed << " / 120 apprenticeships and solo keepers complete; " << total
              << " duels; " << wins << " wins; " << seconds / total << " mean simulation seconds; "
              << locked << " locked casts; " << failures << " blocked\n";
    return failures || locked ? 1 : 0;
}

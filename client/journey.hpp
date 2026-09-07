#pragma once
#include "creature/campaign.hpp"
#include "journey_audio.hpp"
#include "tinikami.hpp"
#include <chrono>
#include <functional>
#include <iomanip>
#include <queue>
namespace journey {
namespace camp = creature::campaign;
using tinikami::bar;
using tinikami::frame;
using tinikami::gold;
using tinikami::ink;
using tinikami::jade;
using tinikami::moss;
using tinikami::muted;
using tinikami::night;
using tinikami::pale;
using tinikami::paper;
using tinikami::tag;
struct Options {
    int frames = 0, scene = 0, region = 0, species = 0, focus_site = -1;
    uint32_t seed = 42;
    bool test = false, seeded = false;
    std::string captures = "captures";
    std::filesystem::path save_path;
};
enum Screen {
    Title,
    Starters,
    Explore,
    Dialogue,
    Party,
    Atlas,
    Journal,
    Battle,
    Result,
    Bells,
    FinalChoice,
    Credits,
    Lantern,
    Studio,
    Inn,
    Notes,
    Help
};
struct Palette {
    SDL_Color floor, path, water, edge, accent;
    int tree, house;
};
constexpr Palette Palettes[8] = {{{94, 117, 91, 255},
                                  {163, 154, 116, 255},
                                  {57, 94, 101, 255},
                                  {61, 83, 66, 255},
                                  {199, 174, 109, 255},
                                  8,
                                  0},
                                 {{83, 112, 101, 255},
                                  {139, 146, 113, 255},
                                  {45, 86, 107, 255},
                                  {50, 79, 77, 255},
                                  {151, 199, 189, 255},
                                  15,
                                  1},
                                 {{120, 132, 102, 255},
                                  {176, 163, 126, 255},
                                  {78, 119, 131, 255},
                                  {81, 101, 82, 255},
                                  {229, 192, 120, 255},
                                  13,
                                  0},
                                 {{105, 123, 95, 255},
                                  {171, 151, 122, 255},
                                  {65, 104, 100, 255},
                                  {69, 92, 71, 255},
                                  {234, 184, 173, 255},
                                  9,
                                  0},
                                 {{117, 99, 88, 255},
                                  {165, 133, 105, 255},
                                  {104, 65, 55, 255},
                                  {74, 70, 64, 255},
                                  {238, 168, 91, 255},
                                  11,
                                  2},
                                 {{156, 179, 179, 255},
                                  {195, 197, 176, 255},
                                  {83, 132, 151, 255},
                                  {104, 134, 141, 255},
                                  {217, 235, 233, 255},
                                  10,
                                  7},
                                 {{91, 107, 129, 255},
                                  {150, 153, 162, 255},
                                  {43, 65, 95, 255},
                                  {54, 71, 101, 255},
                                  {185, 192, 230, 255},
                                  13,
                                  7},
                                 {{111, 115, 132, 255},
                                  {177, 168, 167, 255},
                                  {60, 64, 90, 255},
                                  {70, 72, 94, 255},
                                  {234, 211, 176, 255},
                                  12,
                                  3}};
uint32_t hash2(int a, int b, int c) {
    uint32_t n = uint32_t(a) * 374761393u + uint32_t(b) * 668265263u + uint32_t(c) * 2246822519u;
    n = (n ^ (n >> 13)) * 1274126177u;
    return n ^ (n >> 16);
}
SDL_Color grade(SDL_Color c, int amount) {
    return {uint8_t(std::clamp(int(c.r) + amount, 0, 255)),
            uint8_t(std::clamp(int(c.g) + amount, 0, 255)),
            uint8_t(std::clamp(int(c.b) + amount, 0, 255)), c.a};
}
std::vector<std::string> wrap(const std::string &input, int width = 68) {
    std::vector<std::string> lines;
    std::istringstream in(input);
    std::string word, line;
    while (in >> word) {
        if (int(line.size() + word.size() + 1) > width) {
            lines.push_back(line);
            line.clear();
        }
        if (!line.empty())
            line += ' ';
        line += word;
    }
    if (!line.empty())
        lines.push_back(line);
    return lines;
}
std::vector<std::string> paginate(std::string value) {
    std::vector<std::string> result;
    size_t from = 0;
    do {
        size_t end = value.find('|', from);
        auto lines = wrap(value.substr(from, end == std::string::npos ? end : end - from));
        for (size_t i = 0; i < lines.size(); i += 4) {
            std::string page;
            for (size_t j = i; j < std::min(i + 4, lines.size()); ++j) {
                if (j != i)
                    page += '\n';
                page += lines[j];
            }
            result.push_back(page);
        }
        if (end == std::string::npos)
            break;
        from = end + 1;
    } while (from < value.size());
    if (result.empty())
        result.push_back("");
    return result;
}
const char *kind_name(int kind) {
    static const char *names[] = {"SANCTUARY",       "CONVERSATION", "WILD SPIRIT", "ROAD TRIAL",
                                  "SUPPLIES",        "BELL PUZZLE",  "KEEPER",      "ONWARD",
                                  "ENDURANCE RELAY", "MEMORY"};
    return names[std::clamp(kind, 0, 9)];
}
class Game {
    SDL_Window *window;
    Brain &brain;
    Options options;
    tinikami::Atlas props, cover, biomes, interior;
    journey_audio::Sound sound;
    camp::State state;
    camp::Encounter match;
    Screen screen = Title, return_screen = Explore;
    bool running = true, manual = false, paused = false, won = false, has_save = false,
         preview = false;
    bool art_ready = false, used_remedy = false, show_hitboxes = false, studio_cycle = true;
    int studio_pose = 0, studio_direction = 0;
    int selected = 0, book_page = 0, party_slot = 0, dialog_page = 0, dialog_next = 0, site = 0;
    int battle_round = 0, battle_slot = 0, enemy_hp = 1000, pending_ability = 0, wind_power = 100;
    int toast_age = 0, outcome_age = 0, present = 0, walk_pose = 0, facing = 0, bell_count = 0;
    double camera_x = 0, camera_y = 0, accumulator = 0, save_clock = 0, played_clock = 0;
    std::array<int, 3> vitality{}, bells{};
    std::array<BrainMemory, 2> memories{};
    World duel;
    std::string message, dialog_speaker;
    std::vector<std::string> dialog;
    std::vector<Vec> route;
    int route_site = -1;
    static constexpr int Tile = 48, Top = 86, Bottom = 706;
    void tell(const std::string &who, const std::string &text, int next = 0) {
        dialog_speaker = who;
        dialog = paginate(text);
        dialog_page = 0;
        dialog_next = next;
        screen = Dialogue;
        sound.chime();
    }
    void persist(bool announce = false) {
        if (preview)
            return;
        std::string error;
        if (!camp::save(state, options.save_path, error)) {
            message = "SAVE FAILED: " + error;
            toast_age = 240;
        } else {
            has_save = true;
            if (announce) {
                message = "JOURNEY SAVED";
                toast_age = 120;
            }
        }
    }
    int near_site() const {
        int best = -1;
        int64_t dist = 650 * 650;
        const auto &places = camp::Regions[state.region].sites;
        for (int i = 0; i < camp::SiteCount; i++) {
            int dx = state.x - places[i].x * 256, dy = state.y - places[i].y * 256;
            int64_t d = int64_t(dx) * dx + int64_t(dy) * dy;
            if (d < dist) {
                best = i;
                dist = d;
            }
        }
        return best;
    }
    void place_prop(int id, int x, int y, int size, int alpha = 255) {
        if (id >= 32)
            biomes.draw(id - 32, x - size / 2, y - size * 150 / 160, size, size, 0, false, alpha);
        else
            props.draw(id, x - size / 2, y - size * 118 / 128, size, size, 0, false, alpha);
    }
    void set_region() {
        route.clear();
        route_site = -1;
        sound.region.store(state.region);
        camera_x = state.x * Tile / 256. - 550;
        camera_y = state.y * Tile / 256. - 300;
    }
    void begin(int starter) {
        // Starting a new journey archives the previous slot before any autosave.
        if (!preview && std::filesystem::exists(options.save_path)) {
            std::error_code ec;
            auto archive = options.save_path;
            archive += ".retired-" +
                       std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            std::filesystem::copy_file(options.save_path, archive,
                                       std::filesystem::copy_options::none, ec);
            if (ec) {
                message = "Could not archive previous journey: " + ec.message();
                screen = Title;
                return;
            }
        }
        uint32_t seed = options.seed;
        if (!options.seeded && !options.test) {
            auto time = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
            seed = hash2(int(time), int(time >> 32), starter);
        }
        state = camp::new_journey(seed, starter);
        set_region();
        persist();
        tell("NARA / THE FIRST PAGE", camp::Regions[0].arrival, 0);
    }
    void interact(int id) {
        site = id;
        const auto &node = camp::Regions[state.region].sites[id];
        if (!camp::available(state, state.region, id)) {
            tell(node.speaker, std::string("This part of the road is waiting for you to visit ") +
                                   camp::Regions[state.region].sites[node.prerequisite].name +
                                   " first.");
            return;
        }
        bool done = state.cleared[state.region * 20 + id];
        if (node.kind == camp::Sanctuary) {
            camp::rest(state);
            persist();
            screen = Inn;
            return;
        }
        if (node.kind == camp::Gate) {
            state.cleared[state.region * 20 + id] = 1;
            persist();
            if (state.region == 7) {
                if (state.ending)
                    screen = Credits;
                else
                    screen = FinalChoice;
                return;
            }
            camp::travel(state, state.region + 1);
            set_region();
            persist();
            tell(camp::Regions[state.region].name, camp::Regions[state.region].arrival);
            return;
        }
        if (node.kind == camp::Expedition) {
            tell(node.speaker,
                 "LANTERN WALK / Eight rooms unfold beyond the restored bell. At every fork, "
                 "choose one encounter or a harder two-spirit relay. Your field book travels "
                 "with you: invite fresh friends at each fork. Each spirit keeps its condition; "
                 "two campfires can offer a rest for two threads.|The final room is a "
                 "three-spirit relay. You keep earned threads if you leave early, and the "
                 "completed walk earns a regional seal. The road saves between rooms. This is "
                 "optional, and the main journey remains open.",
                 3);
            return;
        }
        if (node.kind == camp::Puzzle) {
            if (done) {
                tell(node.name, node.after);
                return;
            }
            bell_count = 0;
            screen = Bells;
            return;
        }
        if (node.kind == camp::Wild || node.kind == camp::Trial || node.kind == camp::Keeper ||
            node.kind == camp::Expedition) {
            tell(node.speaker,
                 std::string(node.text) +
                     (node.kind == camp::Wild
                          ? "|Your next encounter is with " +
                                std::string(Roster[node.species].name) +
                                ". Trust: " + num(state.companions[node.species].trust) + " / " +
                                num(camp::trust_needed(node.species)) + "."
                          : "|This is a " +
                                num(node.kind == camp::Keeper       ? 3
                                    : node.kind == camp::Expedition ? 5
                                                                    : 2) +
                                "-spirit relay. The party keeps its condition between rounds. "
                                "Three remedies are available after resting. M switches between "
                                "your spirit's pilot and direct control. Retreat with Escape."),
                 1);
            return;
        }
        bool fresh = !done;
        bool completed = camp::finish_story(state, id);
        persist();
        if (id == 8 && fresh && completed)
            tell(node.speaker, std::string("REQUEST FULFILLED / ") + node.after);
        else
            tell(node.speaker, fresh ? node.text : node.after);
    }
    void start_round() {
        camp::initialize_round(duel, state, match, battle_round, battle_slot, vitality[battle_slot],
                               enemy_hp);
        memories = {};
        pending_ability = 0;
        outcome_age = 0;
        used_remedy = false;
        paused = false;
        accumulator = 0;
        screen = Battle;
        sound.battle.store(true);
    }
    void start_match() {
        match = state.walk_region >= 0 ? camp::walk_encounter(state, state.walk_choice)
                                       : camp::encounter(state, site);
        battle_round = 0;
        enemy_hp = 1000;
        for (int i = 0; i < 3; ++i)
            vitality[i] = state.party[i] < 0 ? 0 : state.companions[state.party[i]].vitality;
        battle_slot = state.lead;
        if (vitality[battle_slot] <= 0) {
            for (int i = 0; i < 3; ++i)
                if (vitality[i] > 0) {
                    battle_slot = i;
                    break;
                }
        }
        if (vitality[battle_slot] <= 0) {
            tell("A QUIET MOMENT",
                 "Your companions need to rest. Return to the sanctuary; recovery is always free.");
            return;
        }
        persist();
        start_round();
    }
    void finish_match(bool victory, bool withdrew = false) {
        won = victory;
        camp::resolve(state, match, won, vitality, withdrew);
        persist();
        set_region();
        screen = Result;
        sound.battle.store(false);
        sound.chime();
    }
    void next_round() {
        bool victory = duel.winner == 0;
        int own = state.party[battle_slot];
        vitality[battle_slot] =
            victory ? std::clamp(duel.bodies[0].hp * 1000 / Roster[own].hp, 1, 1000) : 0;
        if (victory) {
            ++battle_round;
            enemy_hp = 1000;
            if (battle_round >= match.rounds) {
                finish_match(true);
                return;
            }
            // Short between-round recovery makes relays a team exercise, not attrition grinding.
            vitality[battle_slot] = std::min(
                1000, vitality[battle_slot] + (state.companions[own].charm == 3 ? 300 : 150));
        } else {
            enemy_hp = std::max(1, duel.bodies[1].hp * 1000 / Roster[duel.bodies[1].species].hp);
            int next = -1;
            for (int j = 1; j <= 3; j++) {
                int p = (battle_slot + j) % 3;
                if (vitality[p] > 0) {
                    next = p;
                    break;
                }
            }
            if (next < 0) {
                finish_match(false);
                return;
            }
            battle_slot = next;
        }
        start_round();
    }
    static Personality companion_personality(const camp::Companion &c) {
        auto base = personality_preset(c.temperament), variation = personality_from_seed(c.seed);
        for (int i = 0; i < 3; ++i)
            base[i] = std::clamp(base[i] + variation[i] * .15f, -1.f, 1.f);
        return base;
    }
    void advance_battle() {
        if (duel.terminal || duel.truncated) {
            outcome_age += 3;
            if (outcome_age >= 45)
                next_round();
            return;
        }
        std::array<Action, 2> actions;
        actions[0] =
            brain.ready()
                ? brain.action(observe(duel, 0), memories[0],
                               companion_personality(state.companions[state.party[battle_slot]]))
                : scripted(duel, 0);
        actions[1] = brain.ready() && camp::trained_opponent(state, match)
                         ? brain.action(observe(duel, 1), memories[1],
                                        personality_preset((match.region + battle_round) % 5))
                         : scripted(duel, 1, match.styles[battle_round]);
        if (manual) {
            const auto *keys = SDL_GetKeyboardState(nullptr);
            int mx, my;
            float lx, ly;
            SDL_GetMouseState(&mx, &my);
            SDL_RenderWindowToLogical(r, mx, my, &lx, &ly);
            Vec aim{int((lx - AX) * Q / S) - duel.bodies[0].pos.x,
                    int((ly - AY) * Q / S) - duel.bodies[0].pos.y};
            int strength = Q;
            if (pending_ability >= 1 && pending_ability <= 4) {
                auto &m = move_for(duel.bodies[0], pending_ability - 1);
                if (m.wind_strength)
                    strength = wind_power * Q / 100;
                else if (m.kind == Field || m.kind == Trap || m.kind == Turret)
                    strength = std::min(Q, length(aim) * Q / std::max(1, m.range));
            }
            aim = scale(unit(aim), strength);
            actions[0] = {(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * Q,
                          (keys[SDL_SCANCODE_S] - keys[SDL_SCANCODE_W]) * Q, aim.x, aim.y,
                          pending_ability};
        }
        pending_ability = 0;
        step(duel, actions);
    }
    void walk_to(int mx, int my) {
        Vec target{int((mx + camera_x) / Tile), int((my - Top + camera_y) / Tile)};
        bool mini = mx >= 890 && mx < 1082 && my >= 532 && my < 676;
        if (mini)
            target = {(mx - 890) / 3, (my - 532) / 3};
        route_site = -1;
        int closest = mini ? 5 : 3;
        for (int i = 0; i < 20; ++i) {
            const auto &node = camp::Regions[state.region].sites[i];
            int d = (target.x - node.x) * (target.x - node.x) +
                    (target.y - node.y) * (target.y - node.y);
            if (d < closest) {
                closest = d;
                route_site = i;
                target = {node.x, node.y};
            }
        }
        if (!camp::passable(state.region, target.x, target.y)) {
            route_site = -1;
            return;
        }
        constexpr int size = camp::MapWidth * camp::MapHeight;
        std::array<int, size> parent;
        parent.fill(-1);
        int start = state.y / 256 * camp::MapWidth + state.x / 256,
            goal = target.y * camp::MapWidth + target.x;
        std::queue<int> open;
        open.push(start);
        parent[start] = start;
        while (!open.empty() && parent[goal] < 0) {
            int at = open.front();
            open.pop();
            Vec pos{at % camp::MapWidth, at / camp::MapWidth};
            for (Vec d : {Vec{1, 0}, Vec{-1, 0}, Vec{0, 1}, Vec{0, -1}}) {
                auto next = pos + d;
                if (!camp::passable(state.region, next.x, next.y))
                    continue;
                int n = next.y * camp::MapWidth + next.x;
                if (parent[n] < 0) {
                    parent[n] = at;
                    open.push(n);
                }
            }
        }
        route.clear();
        if (parent[goal] < 0) {
            route_site = -1;
            return;
        }
        for (int at = goal; at != start; at = parent[at])
            route.push_back({(at % camp::MapWidth) * 256 + 128, (at / camp::MapWidth) * 256 + 128});
        if (route.empty() && route_site >= 0) {
            int id = route_site;
            route_site = -1;
            interact(id);
        }
    }
    void world(double dt) {
        const auto *keys = SDL_GetKeyboardState(nullptr);
        int dx = keys[SDL_SCANCODE_D] + keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_A] -
                 keys[SDL_SCANCODE_LEFT];
        int dy = keys[SDL_SCANCODE_S] + keys[SDL_SCANCODE_DOWN] - keys[SDL_SCANCODE_W] -
                 keys[SDL_SCANCODE_UP];
        dx = std::clamp(dx, -1, 1);
        dy = std::clamp(dy, -1, 1);
        bool guided = false;
        if (dx || dy) {
            route.clear();
            route_site = -1;
        } else if (!route.empty()) {
            guided = true;
            auto to = route.back();
            int speed = std::max(1, int(dt * 900));
            int ex = to.x - state.x, ey = to.y - state.y;
            dx = (ex > 0) - (ex < 0);
            dy = (ey > 0) - (ey < 0);
            state.x += std::clamp(ex, -speed, speed);
            state.y += std::clamp(ey, -speed, speed);
            if (state.x == to.x && state.y == to.y) {
                route.pop_back();
                if (route.empty() && route_site >= 0) {
                    int id = route_site;
                    route_site = -1;
                    interact(id);
                }
            }
        }
        if (dx || dy) {
            int amount = std::max(
                1, int(dt * (keys[SDL_SCANCODE_LSHIFT] ? 1150 : 760) / (dx && dy ? 1.4142 : 1.)));
            int nx = state.x + dx * amount, ny = state.y + dy * amount;
            if (!guided && camp::passable(state.region, nx / 256, state.y / 256))
                state.x = nx;
            if (!guided && camp::passable(state.region, state.x / 256, ny / 256))
                state.y = ny;
            facing = std::abs(dx) > 0 ? (dx > 0 ? 1 : 3) : (dy > 0 ? 0 : 2);
            walk_pose = 1 + (present / 9) % 2;
        } else
            walk_pose = 0;
        double target_x = state.x * Tile / 256. - 550, target_y = state.y * Tile / 256. - 300;
        camera_x += (target_x - camera_x) * std::min(1., dt * 10);
        camera_y += (target_y - camera_y) * std::min(1., dt * 10);
        camera_x = std::clamp(camera_x, 0., double(camp::MapWidth * Tile - 1100));
        camera_y = std::clamp(camera_y, 0., double(camp::MapHeight * Tile - (Bottom - Top)));
    }
    void draw_world() {
        const auto &p = Palettes[state.region];
        const auto &region = camp::Regions[state.region];
        rect(0, 0, 1100, 780, night);
        SDL_Rect clip{0, Top, 1100, Bottom - Top};
        SDL_RenderSetClipRect(r, &clip);
        int cx = int(camera_x), cy = int(camera_y);
        auto px = [&](int x) { return x * Tile - cx; };
        auto py = [&](int y) { return y * Tile - cy + Top; };
        for (int y = std::max(0, cy / Tile);
             y < std::min(camp::MapHeight, (cy + Bottom - Top) / Tile + 2); ++y)
            for (int x = std::max(0, cx / Tile);
                 x < std::min(camp::MapWidth, (cx + 1100) / Tile + 2); ++x) {
                int type = camp::ground(state.region, x, y);
                auto c = type == 0                ? p.floor
                         : type == 1 || type == 4 ? p.path
                         : type == 3              ? p.edge
                                                  : p.water;
                int variation = int(hash2(x / 5, y / 5, state.region) % 7) - 3;
                rect(px(x), py(y), Tile, Tile, grade(type == 1 ? p.floor : c, variation));
                if (type == 1 || type == 4) {
                    // Rounded path ends and shoulders soften the grid without noisy terrain.
                    int xx = px(x), yy = py(y), radius = 12;
                    auto road = [&](int a, int b) {
                        int t = camp::ground(state.region, a, b);
                        return t == 1 || t == 4;
                    };
                    bool left = road(x - 1, y), right = road(x + 1, y);
                    bool up = road(x, y - 1), down = road(x, y + 1);
                    rect(xx + radius, yy, Tile - 2 * radius, Tile, p.path);
                    rect(xx, yy + radius, Tile, Tile - 2 * radius, p.path);
                    for (int a : {0, 1})
                        for (int b : {0, 1}) {
                            int ox = a ? Tile - radius : radius, oy = b ? Tile - radius : radius;
                            if ((a ? right : left) || (b ? down : up))
                                rect(xx + (a ? Tile - radius : 0), yy + (b ? Tile - radius : 0),
                                     radius, radius, p.path);
                            else
                                circle(xx + ox, yy + oy, radius, p.path, true);
                        }
                }
                if (type == 4) {
                    int xx = px(x), yy = py(y);
                    bool horizontal = camp::ground(state.region, x - 1, y) == 4 ||
                                      camp::ground(state.region, x + 1, y) == 4;
                    rect(xx, yy, Tile, Tile, grade(p.path, -12));
                    for (int step = 0; step < Tile; step += 8) {
                        if (horizontal)
                            line(xx + step, yy + 3, xx + step, yy + Tile - 3, grade(p.path, -34));
                        else
                            line(xx + 3, yy + step, xx + Tile - 3, yy + step, grade(p.path, -34));
                    }
                    if (horizontal) {
                        rect(xx, yy, Tile, 3, gold);
                        rect(xx, yy + Tile - 3, Tile, 3, gold);
                    } else {
                        rect(xx, yy, 3, Tile, gold);
                        rect(xx + Tile - 3, yy, 3, Tile, gold);
                    }
                } else if (type == 3) {
                    rect(px(x), py(y) + Tile - 7, Tile, 7, grade(p.edge, -14));
                    line(px(x) + 2, py(y) + 2, px(x) + Tile - 2, py(y) + 2, grade(p.edge, 14));
                } else if (type == 2) {
                    if (hash2(x, y, state.region) % 3 == 0) {
                        int drift = (present / 5 + x * 7) % 23;
                        line(px(x) + drift, py(y) + 18, px(x) + drift + 10, py(y) + 18,
                             grade(c, 12));
                    }
                    if (camp::ground(state.region, x - 1, y) != 2)
                        rect(px(x), py(y), 3, Tile, p.edge);
                    if (camp::ground(state.region, x, y - 1) != 2)
                        rect(px(x), py(y), Tile, 3, p.edge);
                } else if (type == 0 && hash2(x, y, state.region) % 13 == 0) {
                    int xx = px(x) + 17, yy = py(y) + 22;
                    line(xx, yy, xx - 2, yy - 5, grade(c, -10));
                    line(xx, yy, xx + 3, yy - 3, grade(c, -10));
                }
            }
        // The road tells you where to go; large silhouettes live off its quiet floor.
        struct DrawItem {
            int y, kind, id, x, size;
        };
        std::vector<DrawItem> items;
        for (int y = 2; y < 46; y += 2)
            for (int x = 2; x < 62; x += 2) {
                auto h = hash2(x, y, state.region);
                if (h % 3 || camp::ground(state.region, x, y) != 0)
                    continue;
                bool near = false;
                for (const auto &s : region.sites)
                    if (std::abs(x - s.x) < 4 && std::abs(y - s.y) < 4)
                        near = true;
                if (!near)
                    items.push_back(
                        {py(y), 0, 32 + state.region * 4 + 2, px(x), 130 + int(h % 39)});
            }
        // A small inhabited cluster makes each sanctuary a home, not only a menu.
        for (auto v : {Vec{5, 21}, Vec{10, 19}, Vec{13, 28}}) {
            bool near = false;
            for (const auto &site : region.sites)
                if (std::abs(v.x - site.x) < 4 && std::abs(v.y - site.y) < 4)
                    near = true;
            if (!near && camp::ground(state.region, v.x, v.y) != 2)
                items.push_back({py(v.y), 0, 32 + state.region * 4 + 1, px(v.x), 176});
        }
        for (int i = 0; i < 20; i++) {
            const auto &s = region.sites[i];
            int kind = s.kind;
            int id = kind == camp::Sanctuary                           ? 32 + state.region * 4 + 1
                     : kind == camp::Keeper                            ? 32 + state.region * 4
                     : kind == camp::Gate                              ? 3
                     : kind == camp::Cache                             ? 30
                     : kind == camp::Puzzle                            ? 29
                     : kind == camp::Memory                            ? 32 + state.region * 4 + 3
                     : kind == camp::Trial || kind == camp::Expedition ? 2
                     : i == 11                                         ? 25
                     : state.region == 1                               ? 26
                     : state.region == 5                               ? 27
                                                                       : 24;
            items.push_back({py(s.y), kind == camp::Wild ? 1 : 0,
                             kind == camp::Wild ? s.species : id, px(s.x),
                             kind == camp::Wild           ? tinikami::sprite_size(s.species)
                             : kind == camp::Keeper       ? 248
                             : kind == camp::Sanctuary    ? 202
                             : kind == camp::Conversation ? 76
                                                          : 110});
            if (camp::available(state, state.region, i) && !state.cleared[state.region * 20 + i]) {
                int syy = py(s.y) - 38;
                circle(px(s.x), syy - 24, 5, i == camp::objective(state) ? gold : p.accent, true);
                line(px(s.x), syy - 18, px(s.x), syy - 12, p.accent);
            }
        }
        int player_x = state.x * Tile / 256 - cx, player_y = state.y * Tile / 256 - cy + Top;
        items.push_back({player_y, 2, 0, player_x, 76});
        int lead = state.party[state.lead];
        if (lead >= 0)
            items.push_back({player_y + 18, 3, lead, player_x - 34,
                             std::clamp(tinikami::sprite_size(lead), 44, 120)});
        std::stable_sort(items.begin(), items.end(),
                         [](const auto &a, const auto &b) { return a.y < b.y; });
        for (const auto &item : items) {
            if (item.x < -150 || item.x > 1250 || item.y < Top - 150 || item.y > Bottom + 150)
                continue;
            if (item.kind == 0) {
                if (item.id >= 32 && (item.id - 32) % 4 < 2) {
                    circle(item.x, item.y - 5, item.size / 4, {239, 184, 91, 15}, true);
                    circle(item.x, item.y - 5, item.size / 6, {239, 184, 91, 15}, true);
                }
                place_prop(item.id, item.x, item.y, item.size);
            } else if (item.kind == 2) {
                circle(item.x, item.y + 1, 12, {25, 38, 39, 70}, true);
                int id = 16 + facing * 2 + (walk_pose ? present / 9 % 2 : 0);
                place_prop(id, item.x, item.y, 76);
            } else {
                int pose = item.kind == 3 ? walk_pose : (present / 45 + item.id) % 4 == 0 ? 1 : 0;
                Vec look = item.kind == 3 ? (facing == 0   ? Vec{0, Q}
                                             : facing == 1 ? Vec{Q, 0}
                                             : facing == 2 ? Vec{0, -Q}
                                                           : Vec{-Q, 0})
                                          : Vec{0, Q};
                circle(item.x, item.y, std::max(8, item.size / 5), {25, 38, 39, 60}, true);
                tinikami::spirit(item.id, item.x, item.y, item.size, look, pose);
            }
        }
        // Low-frequency drifting motes; the floor stays visually quiet.
        for (int i = 0; i < 14; i++) {
            int x = (i * 191 + present / 5) % 1100,
                y = Top + (i * 73 + present / 9) % (Bottom - Top);
            rect(x, y, 2, 2, {p.accent.r, p.accent.g, p.accent.b, 75});
        }
        int goal = camp::objective(state);
        const auto &destination = region.sites[goal];
        int markerx = px(destination.x), markery = py(destination.y) - 90;
        if (markerx > 15 && markerx < 1085 && markery > Top + 10 && markery < Bottom) {
            circle(markerx, markery, 8, ink, true);
            circle(markerx, markery, 5, gold, true);
        }
        int nearest = near_site();
        if (nearest >= 0) {
            const auto &s = region.sites[nearest];
            std::string name = s.name;
            int w = std::min(720, int(name.size()) * 12 + 58);
            int tipx = std::clamp(player_x - w / 2, 12, 1088 - w);
            frame(tipx, std::min(Bottom - 52, player_y + 32), w, 40, paper);
            label(tipx + 12, std::min(Bottom - 39, player_y + 45), "E  " + name, ink, 2);
        }
        SDL_RenderSetClipRect(r, nullptr);
        draw_minimap();
        frame(12, 10, 1076, 69, paper);
        label(28, 23, region.name, ink, 3);
        label(30, 54, region.subtitle, moss, 1);
        tag(100, 642, 29, 100, "SPIRITS [B]");
        tag(101, 751, 29, 100, "ATLAS [TAB]");
        tag(102, 860, 29, 100, "JOURNAL [J]");
        tag(103, 969, 29, 101, "HELP [?]");
        frame(12, 716, 1076, 54, paper);
        int next = camp::objective(state);
        label(28, 726, "NEXT / " + std::string(region.sites[next].name), ink, 2);
        label(29, 753, "WASD / CLICK WALK    SHIFT HURRY    E INTERACT    F5 SAVE    F8 MUSIC",
              moss, 1);
        label(796, 728, num(state.threads) + " THREADS / " + num(state.herbs) + " REMEDIES", jade,
              1);
        label(796, 748,
              num(camp::collection_count(state)) + " FRIENDS / " +
                  num(camp::restored_count(state)) + " BELLS",
              moss, 1);
    }
    void draw_minimap() {
        const auto &p = Palettes[state.region];
        const int xx = 890, yy = 532, scale = 3;
        frame(xx - 8, yy - 25, 208, 177, paper);
        label(xx, yy - 15, "LOCAL ROAD / GOLD IS NEXT", moss, 1);
        for (int y = 0; y < 48; ++y)
            for (int x = 0; x < 64; ++x) {
                int t = camp::ground(state.region, x, y);
                rect(xx + x * scale, yy + y * scale, scale, scale,
                     t == 2   ? p.water
                     : t == 3 ? p.edge
                     : t == 0 ? p.floor
                              : p.path);
            }
        for (int i = 0; i < 20; ++i) {
            const auto &site = camp::Regions[state.region].sites[i];
            bool done = state.cleared[state.region * 20 + i];
            SDL_Color c = done ? moss : site.kind == camp::Wild ? jade : paper;
            if (i == camp::objective(state))
                c = gold;
            rect(xx + site.x * scale - 2, yy + site.y * scale - 2, 5, 5, ink);
            rect(xx + site.x * scale - 1, yy + site.y * scale - 1, 3, 3, c);
        }
        circle(xx + state.x * scale / 256, yy + state.y * scale / 256, 3, pale, true);
    }
    void shade() {
        rect(0, 0, 1100, 780, {17, 30, 37, 190});
    }
    void title() {
        rect(0, 0, 1100, 780, night);
        cover.draw(0, 0, 0, 1100, 780);
        rect(0, 0, 1100, 210, {15, 29, 37, 145});
        label(77, 50, "TINIKAMI", pale, 8);
        label(81, 123, "THE UNWRITTEN ROAD", {237, 205, 145, 255}, 3);
        label(83, 162, "A JOURNEY OF SMALL SPIRITS AND SECOND CHANCES", paper, 1);
        frame(67, 486, 470, 232, paper);
        label(88, 507, "THE ROAD IS WAITING.", ink, 2);
        if (has_save)
            tag(1, 89, 546, 424, "CONTINUE / " + std::string(camp::Regions[state.region].name));
        else
            label(91, 552, "EIGHT SANCTUARIES. FORTY SPIRITS. ONE OPEN DOOR.", moss, 1);
        tag(2, 89, 586, 424,
            has_save ? "BEGIN ANOTHER JOURNEY (ARCHIVES CURRENT SAVE)" : "BEGIN YOUR JOURNEY");
        tag(103, 89, 626, 154, "HOW TO PLAY");
        tag(950, 252, 626, 156, "SPRITE STUDIO");
        tag(3, 417, 626, 96, "QUIT");
        label(91, 680, "CAMPAIGN ALPHA / 0.10   MUSIC: F8", moss, 1);
        if (!message.empty())
            label(80, 745, message.substr(0, 145), pale, 1);
    }
    void starters() {
        title();
        buttons.clear();
        shade();
        frame(55, 92, 990, 614, paper);
        label(88, 125, "WHO WILL WALK WITH YOU?", ink, 3);
        auto intro =
            wrap("A spirit joins because it wants to. Your first companion is a beginning, not a "
                 "class restriction. Every species can be befriended along the road.",
                 76);
        int yy = 174;
        for (auto &l : intro) {
            label(88, yy, l, moss, 2);
            yy += 22;
        }
        const int ids[3] = {0, 6, 35};
        const char *text[3] = {"A BOLD FLAME / BURN AND BURST", "QUICK FOOTWORK / ICE AND SPACE",
                               "A GENTLE CURRENT / CLEANSE AND RETURN"};
        for (int i = 0; i < 3; i++) {
            int x = 86 + i * 313;
            frame(x, 272, 295, 352, pale);
            tinikami::spirit(ids[i], x + 146, 450, 150, {0, Q}, present / 18 % 3);
            label(x + 24, 480, Roster[ids[i]].name, ink, 3);
            label(x + 15, 523, text[i], moss, 1);
            tag(10 + i, x + 22, 564, 250, "WALK WITH " + std::string(Roster[ids[i]].name));
        }
        tag(104, 85, 650, 180, "BACK");
    }

    void draw_dialog() {
        draw_world();
        buttons.clear();
        shade();
        frame(90, 424, 920, 298, paper);
        const auto &node = camp::Regions[state.region].sites[std::clamp(site, 0, 19)];
        if (node.kind == camp::Wild &&
            dialog_speaker.find(Roster[node.species].name) != std::string::npos)
            tinikami::spirit(node.species, 146, 426, 120, {0, Q}, present / 30 % 3);
        else
            place_prop(node.kind == camp::Puzzle                           ? 29
                       : dialog_speaker.find("Sable") != std::string::npos ? 25
                       : state.region == 1                                 ? 26
                       : state.region == 5                                 ? 27
                                                                           : 24,
                       146, 426, 110);
        label(117, 446, dialog_speaker, ink, 2);
        line(117, 476, 981, 476, muted);
        label(117, 500, dialog[dialog_page], ink, 2);
        label(117, 680, num(dialog_page + 1) + " / " + num(int(dialog.size())), moss, 1);
        tag(110, 724, 665, 255,
            dialog_page + 1 < int(dialog.size()) ? "CONTINUE [ENTER]"
            : dialog_next == 1                   ? "ENTER THE GARDEN"
                                                 : "CLOSE THE BOOK [ENTER]");
        if (dialog_next == 1)
            tag(104, 445, 665, 260, "LEAVE FOR NOW [ESC]");
    }
    void party() {
        draw_world();
        buttons.clear();
        shade();
        frame(34, 92, 1032, 616, paper);
        label(60, 112, "COMPANIONS", ink, 3);
        label(62, 153, "SELECT A SPIRIT, THEN CHOOSE ITS PLACE. ALL FORTY HAVE A HOME HERE.", moss,
              1);
        for (int j = 0; j < 3; j++) {
            int x = 61 + j * 337;
            int id = state.party[j];
            frame(x, 178, 317, 83, j == party_slot ? pale : paper);
            if (id >= 0) {
                tinikami::spirit(id, x + 36, 245, 60, {0, Q});
                label(x + 75, 192, Roster[id].name, ink, 2);
                bar(x + 76, 223, 163, state.companions[id].vitality, 1000, jade);
            }
            tag(200 + j, x + 225, 229, 80, j == state.lead ? "LEAD" : "SET LEAD", j == state.lead);
            buttons.push_back({{x, 178, 210, 83}, "PARTY SLOT", 210 + j});
        }
        for (int n = 0; n < 20; n++) {
            int id = book_page * 20 + n, x = 61 + (n % 5) * 128, y = 280 + (n / 5) * 84;
            bool owned = camp::befriended(state, id);
            frame(x, y, 119, 78, id == selected ? pale : paper);
            tinikami::spirit(id, x + 59, y + 55, 55, {0, Q}, 0, owned ? 255 : 75);
            if (owned) {
                rect(x + 6, y + 56, 106, 4, moss);
                rect(x + 6, y + 56, 106 * state.companions[id].vitality / 1000, 4, jade);
            }
            label(x + 6, y + 64, Roster[id].name, owned ? ink : muted, 1);
            buttons.push_back({{x, y, 119, 78}, "SPIRIT", 300 + id});
        }
        int x = 731;
        const auto &c = state.companions[selected];
        bool owned = camp::befriended(state, selected);
        label(x, 287, Roster[selected].name, ink, 3);
        label(x, 324, Roster[selected].role, moss, 1);
        label(x, 351, "TRUST " + num(c.trust) + " / " + num(camp::trust_needed(selected)), jade, 2);
        label(x, 380, "BOND RANK " + num(camp::rank(c)) + " / 5", moss, 2);
        bar(x, 408, 270, c.experience % 120, 120, gold);
        label(x, 431, "TEMPERAMENT / " + std::string(personality_name(c.temperament)), moss, 1);
        tag(220, x, 452, 270, "CHANGE TEMPERAMENT", owned);
        static const char *charms[] = {
            "OPEN HAND / FULL ENERGY",           "STONE / 12 SHIELD, 85 ENERGY",
            "WIND / HASTE, 85 ENERGY",           "REED / MORE RELAY HEAL, 85 ENERGY",
            "BELL / BRIEF CC RESIST, 80 ENERGY", "LANTERN / 24 SHIELD, 65 ENERGY"};
        label(x, 496, charms[c.charm], moss, 1);
        tag(221, x, 516, 270,
            camp::rank(c) >= 2 ? "CHANGE CHARM / REQUESTS UNLOCK MORE" : "CHARMS UNLOCK AT BOND 2",
            owned && camp::rank(c) >= 2);
        tag(222, x, 561, 270,
            owned ? "ASSIGN TO SELECTED PARTY SLOT" : "NEEDS TRUST / FIND IN THE WORLD", owned);
        if (!owned && c.trust > 0)
            tag(223, x, 601, 270, "OFFER 3 THREADS / +1 TRUST");
        tag(230, 62, 650, 122, "PREV PAGE");
        tag(231, 196, 650, 122, "NEXT PAGE");
        label(344, 660, "PAGE " + num(book_page + 1) + " / 2", moss, 1);
        tag(224, 505, 650, 256, "ARTS AND FIELD NOTES");
        tag(104, 829, 650, 210, "RETURN");
    }
    void atlas() {
        rect(0, 0, 1100, 780, night);
        cover.draw(0, 0, 0, 1100, 780);
        shade();
        frame(26, 18, 1048, 71, paper);
        label(48, 37, "THE EIGHTFOLD ROAD", ink, 3);
        label(705, 43, "CHOOSE AN OPEN SANCTUARY TO TRAVEL", moss, 1);
        for (int i = 0; i < 8; ++i) {
            int x = 45 + (i % 2) * 520, y = 112 + (i / 2) * 139;
            bool open = camp::accessible(state, i);
            frame(x, y, 488, 119, open ? paper : night);
            label(x + 20, y + 17, num(i + 1) + " / " + camp::Regions[i].name, open ? ink : muted,
                  2);
            label(x + 20, y + 48, camp::Regions[i].subtitle, open ? moss : muted, 1);
            tag(400 + i, x + 20, y + 76, 448,
                open ? (state.cleared[i * 20 + 16] ? "RESTORED / TRAVEL" : "OPEN / TRAVEL")
                     : "RESTORE THE PREVIOUS BELL TO OPEN",
                i == state.region);
        }
        tag(104, 840, 720, 216, "BACK TO THE ROAD");
    }
    void journal() {
        draw_world();
        buttons.clear();
        shade();
        frame(55, 92, 990, 616, paper);
        label(80, 115, "FIELD JOURNAL", ink, 3);
        const auto &rr = camp::Regions[state.region];
        label(82, 157, rr.name, moss, 2);
        for (int i = 0; i < 20; ++i) {
            int x = 82 + (i / 10) * 468, y = 204 + (i % 10) * 40;
            bool done = state.cleared[state.region * 20 + i];
            bool open = camp::available(state, state.region, i);
            label(x, y, done ? "[+]" : open ? "[ ]" : "[-]", done ? jade : muted, 2);
            label(x + 47, y + 1, rr.sites[i].name, done ? moss : ink, 1);
            label(x + 47, y + 16, kind_name(rr.sites[i].kind), muted, 1);
            buttons.push_back({{x, y, 440, 34}, "READ ENTRY", 500 + i});
        }
        label(83, 635, "GOLD MARKERS FOLLOW THE MAIN STORY. EVERY SPIRIT HAS A VISIBLE HOME.", moss,
              1);
        tag(104, 791, 655, 226, "RETURN TO THE ROAD");
    }
    void draw_bells() {
        draw_world();
        buttons.clear();
        shade();
        frame(121, 150, 858, 496, paper);
        label(156, 184, camp::Regions[state.region].sites[site].name, ink, 3);
        const char *names[] = {"LEAF", "TIDE", "EMBER"};
        label(156, 240, "THE INSCRIPTION READS:", moss, 2);
        int row = 278;
        for (const auto &l : wrap(camp::Regions[state.region].sites[site].text, 56)) {
            label(156, row, l, ink, 2);
            row += 23;
        }
        label(156, 376, "RING EACH VOICE ONCE, IN THE ORDER OF THE RIDDLE.", moss, 1);
        for (int i = 0; i < 3; ++i) {
            place_prop(29, 260 + i * 290, 472, 100);
            tag(600 + i, 164 + i * 270, 499, 240, names[i]);
        }
        label(156, 570, "NOTES RUNG: " + num(bell_count) + " / 3", moss, 2);
        tag(104, 735, 594, 214, "LEAVE FOR NOW");
    }
    void battle() {
        std::array<int, 2> personas{state.companions[state.party[battle_slot]].temperament, 0};
        tinikami::draw(duel, {false,
                              manual,
                              false,
                              false,
                              show_hitboxes,
                              0,
                              0,
                              match.weather,
                              1,
                              wind_power,
                              match.seed,
                              "",
                              brain.ready(),
                              false,
                              brain.ready(),
                              personas,
                              {false, false},
                              outcome_age});
        buttons.clear();
        if (duel.terminal || duel.truncated) {
            frame(255, 163, 270, 33, night);
            label(267, 174,
                  duel.winner == 0 ? "YOUR COMPANION PREVAILS" : "THE NEXT COMPANION STEPS FORWARD",
                  pale, 1);
        }
        frame(12, 10, 712, 142, paper);
        const auto &node = camp::Regions[state.region].sites[site];
        label(31, 26, node.name, ink, 3);
        label(32, 62,
              "ROUND " + num(battle_round + 1) + " / " + num(match.rounds) + "   " +
                  std::string(Roster[state.party[battle_slot]].name) + " WALKS WITH YOU",
              moss, 1);
        tag(700, 28, 98, 175,
            manual          ? "YOU PILOT [M]"
            : brain.ready() ? "SPIRIT PILOTS [M]"
                            : "BASIC PILOT [M]",
            !manual);
        tag(701, 213, 98, 150, paused ? "RESUME [P]" : "PAUSE [P]");
        tag(702, 373, 98, 205, "REMEDY [R] / " + num(state.herbs), !used_remedy && state.herbs > 0);
        tag(703, 588, 98, 132, "RETREAT [ESC]");
        frame(738, 10, 330, 73, paper);
        label(750, 20, "PARTY", ink, 1);
        for (int j = 0; j < 3; j++)
            if (state.party[j] >= 0) {
                int x = 766 + j * 98;
                int health = j == battle_slot ? duel.bodies[0].hp * 1000 / Roster[state.party[j]].hp
                                              : vitality[j];
                tinikami::spirit(state.party[j], x + 38, 60, 42, {0, Q});
                bar(x, 64, 81, health, 1000, j == battle_slot ? jade : moss);
            }
        for (int i = 0; i < 2; ++i) {
            const auto &body = duel.bodies[i];
            const auto &species = Roster[body.species];
            int yy = 93 + i * 133;
            frame(738, yy, 330, 125, paper);
            tinikami::spirit(body.species, 785, yy + 95, 90, i ? Vec{-Q, 0} : Vec{Q, 0},
                             body.move >= 0 ? 3 : 0);
            label(833, yy + 12, species.name, i ? tinikami::vermilion : jade, 2);
            label(833, yy + 39, "VITALITY " + num(body.hp) + " / " + num(species.hp), ink, 1);
            bar(833, yy + 53, 218, body.hp, species.hp, moss);
            label(833, yy + 72, "ENERGY " + num(body.energy / 10) + " / 100", jade, 1);
            bar(833, yy + 86, 218, body.energy, 1000, jade, true);
            std::string status = energy_regen(body)
                                     ? "+" + num(energy_regen(body) * 3) + " ENERGY / SEC"
                                 : footwork_load(body) >= 70 ? "FOOTWORK / REGEN LIMITED"
                                                             : "COMMITTED / REGEN PAUSED";
            label(751, yy + 109, status, moss, 1);
        }
        rect(724, 83, 14, 608, night);
        rect(738, 683, 330, 47, night);
        frame(738, 598, 330, 85, paper);
        label(752, 612, "1-4 ARTS / SPACE DODGE / MOUSE AIM", ink, 1);
        label(752, 635, "WASD MOVE / Z-X WIND CAST STRENGTH", moss, 1);
        label(752, 657, "H GEOMETRY / PLANT TO RECOVER", moss, 1);
        frame(20, 691, 1048, 67, paper);
        label(34, 702, "LOTUS CONTROL", ink, 1);
        bar(139, 699, 225, duel.bodies[0].control, 600, jade);
        bar(381, 699, 225, duel.bodies[1].control, 600, tinikami::vermilion);
        label(655, 702,
              "WIND " + num(wind_vector(duel).x) + "," + num(wind_vector(duel).y) + " / CAST " +
                  num(wind_power) + "%",
              moss, 1);
        label(35, 735,
              "SIDEWAYS SPRINTING SPENDS BREATH. STILLNESS RESTORES IT. THE CENTRAL GARDEN CAN WIN "
              "A ROUND.",
              moss, 1);
        for (int j = 0; j < 5; j++)
            buttons.push_back({{747, 399 + j * 36, 312, 33}, "CAST", 710 + j});
        if (paused) {
            rect(AX, AY, 24 * S, 18 * S, {19, 35, 39, 110});
            frame(194, 332, 340, 111, paper);
            label(222, 353, "A MOMENT TO THINK", ink, 2);
            tag(701, 221, 394, 284, "RESUME [P]");
        }
    }
    void result() {
        draw_world();
        buttons.clear();
        shade();
        frame(110, 148, 880, 478, paper);
        label(143, 181, won ? "THE GARDEN GROWS QUIET" : "A PLACE TO BEGIN AGAIN", ink, 3);
        const auto &node = camp::Regions[state.region].sites[site];
        label(143, 239, won ? node.name : "YOUR PARTY HAS RETURNED TO THE SANCTUARY.", moss, 2);
        if (won) {
            bool walk_complete = match.walk && state.walk_region < 0;
            label(143, 285,
                  "+" + num(camp::encounter_reward(state, match) + (walk_complete ? 12 : 0)) +
                      " MEMORY THREADS / BOND EXPERIENCE FOR THE PARTY",
                  jade, 2);
            if (match.walk) {
                label(143, 345, walk_complete ? "A REGIONAL LANTERN SEAL" : "THE NEXT FORK AWAITS",
                      ink, 2);
                label(143, 387,
                      walk_complete ? "EIGHT ROOMS COMPLETE / YOUR PARTY IS RESTED"
                                    : "YOUR COMPANIONS KEEP THEIR CONDITION BETWEEN ROOMS",
                      moss, 1);
                for (int slot = 0; slot < 3; ++slot)
                    if (state.party[slot] >= 0)
                        tinikami::spirit(state.party[slot], 260 + slot * 245, 500, 96, {0, Q}, 0);
            } else if (node.kind == camp::Wild) {
                int sp = node.species;
                selected = sp;
                tinikami::spirit(sp, 225, 474, 145, {0, Q}, 0);
                label(340, 349, Roster[sp].name, ink, 3);
                label(340, 392,
                      "TRUST " + num(state.companions[sp].trust) + " / " +
                          num(camp::trust_needed(sp)),
                      jade, 2);
                if (camp::befriended(state, sp))
                    label(340, 430, "A NEW FRIEND / AVAILABLE IN YOUR SPIRIT BOOK", moss, 1);
                else
                    tag(223, 340, 426, 518, "OFFER 3 THREADS TO GROW TRUST");
            } else {
                auto lines = wrap(
                    "Your companions learned from the encounter. A restored keeper opens the next "
                    "road and recovers the whole party. The journal records what changed.",
                    63);
                int y = 348;
                for (auto &l : lines) {
                    label(143, y, l, moss, 2);
                    y += 24;
                }
            }
        } else {
            label(143, 300, "NO SPIRITS OR THREADS WERE LOST.", jade, 2);
            if (node.kind == camp::Wild && state.companions[node.species].trust > 0)
                label(143, 326, "THE WILD SPIRIT REMEMBERS YOU / TRUST CAN GROW IN THE SPIRIT BOOK",
                      jade, 1);
            auto ls = wrap(
                "Try another party, temperament or charm in the spirit book, or take direct "
                "control with M. You can always rest freely. Wild spirits remain available for "
                "friendly challenges.",
                65);
            int y = 350;
            for (auto &l : ls) {
                label(143, y, l, moss, 2);
                y += 26;
            }
        }
        tag(720, 664, 559, 286,
            won ? (match.walk && state.walk_region >= 0 ? "CONTINUE LANTERN WALK"
                                                        : "READ THE NEXT PAGE")
                : "RETURN TO THE ROAD");
        tag(100, 143, 559, 260, "ARRANGE THE PARTY");
    }
    void final_choice() {
        draw_world();
        buttons.clear();
        shade();
        frame(80, 120, 940, 540, paper);
        label(112, 157, "WHAT SHOULD THE ROAD BECOME?", ink, 3);
        label(112, 215, "HUSH IS FREE. THE BELLS CAN HEAR ONE ANOTHER.", jade, 2);
        auto a = wrap("Keep an open circle of keepers: each village tends its own bell, and every "
                      "promise is renewed together. No keeper can bind a spirit alone.",
                      67);
        int y = 269;
        for (auto &l : a) {
            label(112, y, l, ink, 2);
            y += 25;
        }
        tag(800, 112, 361, 867, "THE OPEN CIRCLE / SHARED KEEPERSHIP");
        auto b =
            wrap("Leave the Crown unbound: the bells answer whoever visits. The valley will have "
                 "to learn to live with weather, change, and spirits who choose to leave.",
                 67);
        y = 421;
        for (auto &l : b) {
            label(112, y, l, ink, 2);
            y += 25;
        }
        tag(801, 112, 517, 867, "THE UNBOUND ROAD / NO PERMANENT KEEPER");
        label(112, 608, "BOTH ENDINGS LEAVE THE WORLD OPEN TO EXPLORE.", moss, 1);
    }

    void credits() {
        rect(0, 0, 1100, 780, night);
        cover.draw(0, 0, 0, 1100, 780);
        shade();
        frame(113, 106, 874, 568, paper);
        label(153, 144, "THE ROAD CONTINUES", ink, 4);
        auto end = state.ending == 1 ? "Every year, the keepers meet beneath the first bell. They "
                                       "ask each spirit whether it wishes to renew its promise. "
                                       "Sometimes the answer is no. The road makes room."
                                     : "The Crown becomes a guest house. Travellers leave their "
                                       "names in a book they are free to take back. Some bells "
                                       "ring only when it rains. People learn to listen for them.";
        auto lines = wrap(end, 61);
        int y = 231;
        for (auto &l : lines) {
            label(153, y, l, moss, 2);
            y += 26;
        }
        label(153, 377, "TINIKAMI / THE UNWRITTEN ROAD", ink, 2);
        label(153, 419, "A GAME BY TINIKAMI CONTRIBUTORS", moss, 2);
        label(153, 460, "ORIGINAL SPIRIT ART / DETERMINISTIC COMBAT / LEARNED PILOTS", moss, 1);
        label(153, 493, "THIS CAMPAIGN IS AN ALPHA. THANK YOU FOR WALKING ITS FIRST ROAD.", moss,
              1);
        label(153, 537,
              num(camp::collection_count(state)) + " / 40 SPIRIT FRIENDS     " +
                  num(state.victories) + " GARDENS SHARED",
              jade, 2);
        tag(104, 153, 595, 356, "CONTINUE EXPLORING");
        tag(802, 535, 595, 408, "RETURN TO TITLE");
    }
    void lantern() {
        draw_world();
        buttons.clear();
        shade();
        frame(60, 105, 980, 568, paper);
        label(91, 136, "THE LANTERN WALK", ink, 4);
        label(92, 187, camp::Regions[state.region].name, moss, 2);
        label(92, 222,
              "ROOM " + num(state.walk_depth + 1) + " / 8     REGIONAL SEALS " +
                  num(state.walks[state.region]),
              jade, 2);
        label(92, 257, "INVITE FRESH FRIENDS AT EACH FORK. INDIVIDUAL CONDITION PERSISTS.", moss,
              1);
        for (int choice = 0; choice < 2; ++choice) {
            auto e = camp::walk_encounter(state, choice);
            int xx = 92 + choice * 462;
            frame(xx, 295, 440, 241, pale);
            label(xx + 21, 315, choice ? "THE RESTLESS PATH" : "THE QUIET PATH", ink, 2);
            label(xx + 21, 350,
                  num(e.rounds) + " SPIRIT" + (e.rounds == 1 ? "" : "S") + " / " +
                      num(e.rounds * (choice ? 3 : 2)) + " THREADS",
                  moss, 1);
            label(xx + 21, 375,
                  std::string(arena_name(e.arena)) + " / " +
                      (e.weather == 0   ? "CLEAR"
                       : e.weather == 1 ? "BREEZE"
                                        : "RAIN") +
                      (choice ? " / TRAINED PILOTS" : ""),
                  jade, 1);
            for (int j = 0; j < e.rounds; ++j) {
                tinikami::spirit(e.enemies[j], xx + 68 + j * 123, 478, 90, {0, Q},
                                 present / 25 % 3);
                label(xx + 27 + j * 123, 483, Roster[e.enemies[j]].name, moss, 1);
            }
            tag(900 + choice, xx + 20, 499, 400,
                choice ? "TAKE THE RESTLESS PATH" : "TAKE THE QUIET PATH");
        }
        if (state.walk_depth == 2 || state.walk_depth == 5)
            tag(902, 92, 559, 438, "CAMPFIRE / 2 THREADS / RECOVER 35%, +1 REMEDY");
        else
            tag(100, 92, 559, 438, "ARRANGE COMPANIONS AND CHARMS");
        tag(903, 552, 559, 438, "RETURN HOME / KEEP WHAT YOU HAVE EARNED");
        label(92, 628,
              "THE FINAL ROOM HOLDS THREE SPIRITS. FINISH TO EARN A SEAL AND 12 EXTRA THREADS.",
              moss, 1);
    }

    void studio() {
        rect(0, 0, 1100, 780, night);
        frame(20, 20, 1060, 740, paper);
        label(47, 43, "THE SPIRIT ANIMATION STUDIO", ink, 3);
        label(49, 84, "40 SPECIES / FOUR DIRECTIONS / EIGHT AUTHORED POSES EACH", moss, 1);
        static const char *poses[] = {"IDLE",   "WALK A", "WALK B", "WINDUP",
                                      "ATTACK", "HIT",    "FALL",   "FAINT"};
        static const char *directions[] = {"SOUTH", "EAST", "NORTH", "WEST"};
        for (int y = 0; y < 4; ++y) {
            label(314, 151 + y * 118, directions[y], moss, 1);
            for (int x = 0; x < 8; ++x) {
                int xx = 318 + x * 91, yy = 170 + y * 118;
                frame(xx, yy, 84, 90, pale);
                tinikami::animations[selected].draw(y * 8 + x, xx + 2, yy + 3, 80, 80);
                if (y == 0)
                    label(xx + 3, 128, poses[x], moss, 1);
                buttons.push_back({{xx, yy, 84, 90}, "POSE", 1000 + y * 8 + x});
            }
        }
        int direction = studio_cycle ? (present / 80) % 4 : studio_direction,
            pose = studio_cycle ? (present / 10) % 8 : studio_pose;
        const Vec facing[] = {Vec{0, Q}, Vec{Q, 0}, Vec{0, -Q}, Vec{-Q, 0}};
        tinikami::spirit(selected, 165, 355, 240, facing[direction], pose);
        label(48, 386, Roster[selected].name, ink, 3);
        label(49, 429, directions[direction], jade, 2);
        label(49, 462, poses[pose], moss, 2);
        tag(951, 48, 514, 230, studio_cycle ? "PAUSE PREVIEW" : "ANIMATE PREVIEW", studio_cycle);
        tag(952, 49, 598, 107, "PREVIOUS");
        tag(953, 165, 598, 112, "NEXT");
        label(317, 621, "WORLD SCALE / SMALL AND LARGE SHARE THE SAME GROUND PLANE", moss, 1);
        for (int i = 0; i < 3; ++i) {
            int sp = i == 0 ? 3 : i == 1 ? selected : 5;
            int xx = 399 + i * 235;
            tinikami::spirit(sp, xx, 720, tinikami::sprite_size(sp), {0, Q}, 0);
            label(xx - 50, 734, Roster[sp].name, moss, 1);
        }
        tag(954, 49, 677, 228, "RETURN TO TITLE");
    }
    void inn() {
        rect(0, 0, 1100, 780, night);
        interior.draw(0, 0, 0, 1100, 780);
        frame(22, 19, 700, 87, paper);
        label(44, 38, camp::Regions[state.region].sites[0].name, ink, 3);
        label(45, 80, "ALL COMPANIONS RESTED / AT LEAST THREE REMEDIES READY", moss, 1);
        for (int j = 0; j < 3; ++j)
            if (state.party[j] >= 0) {
                int id = state.party[j], xx = 309 + j * 134, yy = 578 + (j % 2) * 38;
                circle(xx, yy, 18, {19, 28, 29, 80}, true);
                tinikami::spirit(id, xx, yy, tinikami::sprite_size(id) * 3 / 2, {0, Q},
                                 present / 35 % 3);
            }
        frame(737, 324, 333, 409, paper);
        label(758, 349, "A PLACE TO REST", ink, 2);
        tag(100, 757, 392, 291, "COMPANIONS AND CHARMS");
        tag(960, 757, 434, 291, "CRAFT REMEDY / 2 THREADS");
        tag(961, 757, 476, 291, "TALK WITH THE KEEPER");
        tag(962, 757, 518, 291,
            camp::available(state, state.region, 18) ? "BEGIN A LANTERN WALK"
                                                     : "RESTORE BELL FOR LANTERN WALKS");
        tag(104, 757, 576, 291, "RETURN TO THE ROAD");
        label(759, 636, num(state.threads) + " THREADS / " + num(state.herbs) + " REMEDIES", jade,
              1);
        label(759, 668, "FREE RECOVERY. NO COMPANION IS LOST.", moss, 1);
        frame(24, 671, 678, 73, paper);
        int seals = 0, requests = 0;
        for (int n = 0; n < 8; ++n) {
            seals += state.walks[n] > 0;
            requests += state.cleared[n * 20 + 8] != 0;
        }
        label(44, 688,
              num(camp::collection_count(state)) + " / 40 FRIENDS    " +
                  num(camp::restored_count(state)) + " / 8 BELLS",
              ink, 2);
        label(45, 723,
              num(requests) + " / 8 VILLAGE REQUESTS    " + num(seals) +
                  " / 8 LANTERN SEALS    PLAYED " + num(state.play_seconds / 3600) + "H " +
                  num(state.play_seconds / 60 % 60) + "M",
              moss, 1);
    }
    void notes() {
        draw_world();
        buttons.clear();
        shade();
        frame(29, 28, 1042, 718, paper);
        const auto &sp = Roster[selected];
        label(55, 52, sp.name, ink, 4);
        label(57, 96, sp.role, moss, 1);
        tinikami::spirit(selected, 179, 324, 190, {0, Q}, present / 20 % 3);
        label(58, 352, "VITALITY " + num(sp.hp), jade, 2);
        label(58, 389, "ENERGY +" + num(sp.regen * 3) + " / SEC", moss, 1);
        label(58, 417, "FORWARD / SIDE / REVERSE", moss, 1);
        label(58, 439, "100% / " + num(sp.strafe) + "% / " + num(sp.backward) + "%", ink, 2);
        int yy = 484;
        for (const auto &l : wrap(sp.identity, 23)) {
            label(58, yy, l, ink, 1);
            yy += 17;
        }
        Body body;
        body.species = selected;
        for (int i = 0; i < 4; ++i) {
            const auto &move = move_for(body, i);
            int y = 130 + i * 128;
            frame(344, y, 698, 119, pale);
            tinikami::effects.draw(tinikami::effect_id(move), 358, y + 15, 65, 65);
            label(443, y + 14, num(i + 1) + " / " + move.name, ink, 2);
            label(444, y + 45,
                  "ENERGY " + num(move.cost / 10) + "   COOLDOWN " +
                      tinikami::seconds(move.cooldown) + "S   BASE HIT " + num(move.damage),
                  moss, 1);
            label(444, y + 65,
                  "WINDUP " + tinikami::seconds(move.startup) + "S / RECOVERY " +
                      tinikami::seconds(move.recovery) + "S / " +
                      (move.move_start ? "MOBILE CAST" : "PLANTED CAST"),
                  moss, 1);
            std::string effects;
            if (move.surface)
                effects += "CREATES " + std::string(surface_name(move.surface)) + "  ";
            if (move.wind_strength)
                effects += "WIND " + tinikami::seconds(move.wind_duration) + "S  ";
            if (move.burn)
                effects += "BURN  ";
            if (move.root)
                effects += "ROOT  ";
            if (move.slow)
                effects += "SLOW  ";
            if (move.heal)
                effects += "HEAL  ";
            if (move.shield)
                effects += "SHIELD  ";
            if (move.cleanse)
                effects += "CLEANSE  ";
            if (move.returning)
                effects += "RETURNS  ";
            if (move.poison)
                effects += "POISON  ";
            label(444, y + 91,
                  effects.empty() ? "WATCH ITS DIRECTION, RANGE AND COMMITMENT"
                                  : effects.substr(0, 95),
                  jade, 1);
        }
        tag(225, 58, 684, 260, "BACK TO COMPANIONS");
        label(348, 694, "DODGE USES THE SAME ENERGY POOL. NUMBERS DESCRIBE BASE EFFECTS.", moss, 1);
    }
    void help() {
        if (return_screen == Title)
            title();
        else
            draw_world();
        buttons.clear();
        shade();
        frame(70, 92, 960, 620, paper);
        label(104, 122, "A KEEPER'S FIELD GUIDE", ink, 3);
        const char *lines[] = {"WASD / ARROWS WALK. CLICK A PLACE TO WALK THERE. SHIFT HURRIES.",
                               "E / ENTER INTERACTS WITH THE NEAREST MARKED PLACE.",
                               "GOLD MARKERS AND THE JOURNAL FOLLOW THE MAIN STORY.",
                               "B OPENS YOUR SPIRITS. TAB OPENS THE TRAVEL ATLAS.",
                               "FIRST MEETINGS BUILD TRUST EVEN IN DEFEAT. NO CAPTURE DICE.",
                               "OFFER 3 THREADS AFTER FIRST CONTACT TO DEEPEN TRUST.",
                               "SANCTUARIES RECOVER EVERYONE AND REFILL 3 REMEDIES.",
                               "IN BATTLE, YOUR SPIRIT CAN PILOT ITSELF WITH ITS LEARNED BRAIN.",
                               "M TAKES DIRECT CONTROL. WASD MOVE, MOUSE AIM, 1-4 ARTS.",
                               "SPACE DODGES. P PAUSES. R USES ONE REMEDY PER ROUND.",
                               "Z / X SET WIND CAST STRENGTH. H SHOWS EXACT GEOMETRY.",
                               "HARD STRAFING LIMITS ENERGY RECOVERY. PLANT AND BREATHE.",
                               "BOND 2 UNLOCKS CHARMS. VILLAGE REQUESTS ADD THREE MORE.",
                               "DEFEAT RETURNS YOU HOME. NOTHING IS PERMANENTLY LOST.",
                               "F5 SAVES. F8 TOGGLES THE ORIGINAL GENERATIVE SOUNDTRACK."};
        int y = 179;
        for (auto &l : lines) {
            label(105, y, l, ink, 1);
            y += 28;
        }
        tag(104, 724, 642, 265, "BACK");
    }
    void choose(int id) {
        sound.chime();
        if (id == 3) {
            running = false;
            return;
        }
        if (id == 950) {
            screen = Studio;
            return;
        }
        if (id == 951) {
            studio_cycle = !studio_cycle;
            return;
        }
        if (id == 952 || id == 953) {
            selected = (selected + (id == 952 ? 39 : 1)) % 40;
            return;
        }
        if (id == 954) {
            screen = Title;
            return;
        }
        if (id == 960) {
            if (camp::craft_remedy(state)) {
                persist();
                message = "A FIELD REMEDY IS READY";
            } else
                message = "NEED TWO THREADS AND ROOM IN THE SATCHEL";
            toast_age = 180;
            return;
        }
        if (id == 961) {
            tell(camp::Regions[state.region].sites[0].speaker,
                 state.cleared[state.region * 20 + 16] ? camp::Regions[state.region].epilogue
                                                       : camp::Regions[state.region].sites[0].text,
                 4);
            return;
        }
        if (id == 962) {
            if (camp::available(state, state.region, 18)) {
                site = 18;
                interact(18);
            }
            return;
        }
        if (id >= 1000 && id < 1032) {
            studio_cycle = false;
            studio_direction = (id - 1000) / 8;
            studio_pose = (id - 1000) % 8;
            return;
        }
        if (id == 1) {
            std::string err;
            if (camp::load(state, options.save_path, err)) {
                set_region();
                screen = state.walk_region >= 0 ? Lantern : Explore;
            } else {
                auto back = options.save_path;
                back += ".bak";
                if (camp::load(state, back, err)) {
                    set_region();
                    screen = state.walk_region >= 0 ? Lantern : Explore;
                    message = "RECOVERED BACKUP";
                } else
                    message = err;
            }
            return;
        }
        if (id == 2) {
            screen = Starters;
            return;
        }
        if (id >= 10 && id <= 12) {
            const int ids[] = {0, 6, 35};
            begin(ids[id - 10]);
            return;
        }
        if (id == 100) {
            return_screen = screen;
            screen = Party;
            party_slot = state.lead;
            selected = state.party[state.lead];
            book_page = selected / 20;
            return;
        }
        if (id == 101) {
            screen = Atlas;
            return;
        }
        if (id == 102) {
            screen = Journal;
            return;
        }
        if (id == 103) {
            return_screen = screen;
            screen = Help;
            return;
        }
        if (id == 104) {
            if (screen == Result) {
                choose(720);
                return;
            }
            if (screen == Starters)
                screen = Title;
            else if (screen == Help)
                screen = return_screen;
            else if (screen == Party)
                screen = return_screen;
            else
                screen = Explore;
            return;
        }
        if (id == 110) {
            if (++dialog_page < int(dialog.size()))
                return;
            if (dialog_next == 1)
                start_match();
            else if (dialog_next == 3) {
                if (camp::begin_walk(state)) {
                    persist();
                    screen = Lantern;
                }
            } else if (dialog_next == 4)
                screen = Inn;
            else if (dialog_next == 2 && state.region == 7 && !state.ending)
                screen = FinalChoice;
            else
                screen = Explore;
            return;
        }
        if (id >= 200 && id < 203) {
            int p = id - 200;
            if (state.party[p] >= 0) {
                state.lead = p;
                party_slot = p;
                selected = state.party[p];
                persist();
            }
            return;
        }
        if (id >= 210 && id < 213) {
            party_slot = id - 210;
            return;
        }
        if (id >= 300 && id < 340) {
            selected = id - 300;
            return;
        }
        if (id == 220 && camp::befriended(state, selected)) {
            state.companions[selected].temperament =
                (state.companions[selected].temperament + 1) % 5;
            persist();
            return;
        }
        if (id == 221 && camp::befriended(state, selected) &&
            camp::rank(state.companions[selected]) >= 2) {
            int next = state.companions[selected].charm;
            do {
                next = (next + 1) % 6;
            } while (!camp::charm_unlocked(state, selected, next));
            state.companions[selected].charm = next;
            persist();
            return;
        }
        if (id == 222) {
            if (camp::set_party(state, party_slot, selected))
                persist();
            return;
        }
        if (id == 224) {
            screen = Notes;
            return;
        }
        if (id == 225) {
            screen = Party;
            return;
        }
        if (id == 223) {
            if (camp::offer_thread(state, selected))
                persist();
            else {
                message = "NEED 3 THREADS AND ONE TRUST MARK";
                toast_age = 180;
            }
            return;
        }
        if (id == 230 || id == 231) {
            book_page = 1 - book_page;
            return;
        }
        if (id >= 400 && id < 408) {
            if (camp::travel(state, id - 400)) {
                set_region();
                persist();
                screen = Explore;
            }
            return;
        }
        if (id >= 500 && id < 520) {
            int n = id - 500;
            auto &s = camp::Regions[state.region].sites[n];
            if (state.cleared[state.region * 20 + n])
                tell(s.speaker, std::string(s.text) + "|" + s.after);
            else
                tell("FIELD NOTE",
                     std::string(s.name) + " / " + kind_name(s.kind) + ". " +
                         (camp::available(state, state.region, n)
                              ? "Find the marker in the world to continue."
                              : "First visit " +
                                    std::string(
                                        camp::Regions[state.region].sites[s.prerequisite].name) +
                                    "."));
            return;
        }
        if (id >= 600 && id < 603) {
            bells[bell_count++] = id - 600;
            if (bell_count == 3) {
                if (camp::solve_puzzle(state, site, bells)) {
                    persist();
                    tell("THE BELLS ANSWER", camp::Regions[state.region].sites[site].after);
                } else {
                    bell_count = 0;
                    message = "A NOTE IS OUT OF PLACE. LISTEN AGAIN.";
                    toast_age = 180;
                }
            }
            return;
        }
        if (id == 700) {
            manual = !manual;
            memories[0] = {};
            pending_ability = 0;
            return;
        }
        if (id == 701) {
            paused = !paused;
            return;
        }
        if (id == 702) {
            if (!used_remedy && state.herbs > 0 && !duel.terminal && !duel.truncated) {
                --state.herbs;
                used_remedy = true;
                duel.bodies[0].hp =
                    std::min(Roster[duel.bodies[0].species].hp,
                             duel.bodies[0].hp + Roster[duel.bodies[0].species].hp / 3);
            }
            return;
        }
        if (id == 703) {
            if (screen == Battle && !duel.terminal && !duel.truncated) {
                finish_match(false, true);
            }
            return;
        }
        if (id >= 710 && id < 715) {
            manual = true;
            pending_ability = id - 709;
            return;
        }
        if (id == 720) {
            if (match.walk && won && state.walk_region >= 0) {
                screen = Lantern;
                return;
            }
            if (won) {
                const auto &n = camp::Regions[state.region].sites[site];
                tell(n.speaker, n.after, n.kind == camp::Keeper ? 2 : 0);
            } else
                screen = Explore;
            return;
        }
        if (id == 800 || id == 801) {
            state.ending = id - 799;
            persist();
            screen = Credits;
            return;
        }
        if (id == 802) {
            screen = Title;
            return;
        }
        if (id == 900 || id == 901) {
            state.walk_choice = id - 900;
            site = 18;
            start_match();
            return;
        }
        if (id == 902) {
            if (camp::rest_walk(state))
                persist();
            else {
                message = "THE CAMPFIRE NEEDS TWO THREADS";
                toast_age = 180;
            }
            return;
        }
        if (id == 903) {
            camp::leave_walk(state);
            set_region();
            persist();
            screen = Explore;
            return;
        }
    }

  public:
    Game(SDL_Window *w, Brain &b, const std::filesystem::path &art, Options opts)
        : window(w), brain(b), options(std::move(opts)) {
        preview = options.scene != 0;
        art_ready = interior.load(art / "journey-inn.rgba", 1, 1) &&
                    biomes.load(art / "journey-biomes-atlas.rgba", 4, 8, false, false, true) &&
                    props.load(art / "journey-props-atlas.rgba", 8, 4) &&
                    cover.load(art / "journey-atlas.rgba", 1, 1);
        for (bool ready : tinikami::animated)
            art_ready = art_ready && ready;
        if (options.test)
            options.save_path =
                std::filesystem::temp_directory_path() /
                ("tinikami-ui-test-" +
                 std::to_string(std::chrono::system_clock::now().time_since_epoch().count())) /
                "journey.tini";
        if (options.save_path.empty()) {
            char *dir = SDL_GetPrefPath("Tinikami", "The Unwritten Road");
            options.save_path = std::filesystem::path(dir ? dir : "saves") / "journey.tini";
            SDL_free(dir);
        }
        std::string err;
        has_save = camp::load(state, options.save_path, err);
        if (!has_save) {
            auto backup = options.save_path;
            backup += ".bak";
            has_save = camp::load(state, backup, err);
            if (has_save)
                message = "A BACKUP JOURNEY IS AVAILABLE";
        }
        if (!has_save) {
            state = camp::new_journey(options.seed, 0);
            if (std::filesystem::exists(options.save_path))
                message = "EXISTING SAVE COULD NOT BE READ. A NEW JOURNEY WILL ARCHIVE IT.";
        }
        if (preview) {
            state = camp::new_journey(options.seed, 0);
            for (int rr = 0; rr < options.region; ++rr)
                for (int j : {1, 3, 4, 6, 9, 11, 12, 16, 17})
                    state.cleared[rr * 20 + j] = 1;
            camp::travel(state, options.region);
            for (int s = 0; s < 40; ++s)
                state.companions[s].trust = camp::trust_needed(s);
            state.party = {0, 1, 6};
            if (options.focus_site >= 0) {
                auto &focus = camp::Regions[state.region].sites[options.focus_site];
                state.x = focus.x * 256;
                state.y = (focus.y + 1) * 256;
            }
            if (options.scene == 1)
                screen = Explore;
            else if (options.scene == 2)
                screen = Atlas;
            else if (options.scene == 3)
                screen = Party;
            else if (options.scene == 4) {
                site = 3;
                start_match();
                for (int t = 0; t < 12; ++t)
                    advance_battle();
                paused = false;
            } else if (options.scene == 5) {
                state.ending = 1;
                screen = Credits;
            } else if (options.scene == 6)
                screen = Starters;
            else if (options.scene == 7) {
                for (int j : {1, 3, 4, 6, 9, 11, 12, 16})
                    state.cleared[state.region * 20 + j] = 1;
                camp::begin_walk(state);
                screen = Lantern;
            } else if (options.scene == 8) {
                selected = options.species;
                screen = Studio;
            } else if (options.scene == 9)
                screen = Inn;
            else if (options.scene == 10)
                tell(camp::Regions[state.region].sites[1].speaker,
                     camp::Regions[state.region].sites[1].text);
            else if (options.scene == 11) {
                site = 2;
                match = camp::encounter(state, site);
                won = true;
                screen = Result;
            } else if (options.scene == 12) {
                site = 15;
                screen = Bells;
            } else if (options.scene == 13)
                screen = FinalChoice;
            else if (options.scene == 14)
                screen = Journal;
            else if (options.scene == 15)
                screen = Help;
            else if (options.scene == 16) {
                selected = options.species;
                screen = Notes;
            } else if (options.scene == 17 || options.scene == 18) {
                for (int j : {1, 3, 4, 6, 9, 11, 12, 16})
                    state.cleared[state.region * 20 + j] = 1;
                camp::begin_walk(state);
                if (options.scene == 18)
                    state.walk_depth = 7;
                site = 18;
                match = camp::walk_encounter(state, 0);
                won = true;
                camp::resolve(state, match, true, {800, 600, 400});
                screen = Result;
            }
        }
        set_region();
        if (!options.frames && !options.test)
            sound.start();
        SDL_SetWindowTitle(window, "Tinikami - The Unwritten Road");
        std::cout << "Journey save: " << options.save_path << "\n";
    }
    ~Game() {
        props.free();
        cover.free();
        biomes.free();
        interior.free();
    }
    int controller_test() {
        int checks = 0;
        auto check = [&](bool value, const char *why) {
            ++checks;
            if (!value)
                throw std::runtime_error(why);
        };
        try {
            check(screen == Title && !has_save, "isolated title");
            choose(2);
            check(screen == Starters, "starter selection");
            choose(10);
            check(screen == Dialogue && has_save, "new journey and save");
            while (screen == Dialogue)
                choose(110);
            check(screen == Explore, "intro closes");
            interact(0);
            check(screen == Inn, "sanctuary interior");
            choose(100);
            check(screen == Party, "party opens from inn");
            choose(220);
            choose(104);
            check(screen == Inn, "party returns to inn");
            int herbs = state.herbs, threads = state.threads;
            choose(960);
            check(state.herbs == herbs + 1 && state.threads == threads - 2, "remedy crafting");
            choose(961);
            while (screen == Dialogue)
                choose(110);
            check(screen == Inn, "keeper returns to inn");
            choose(104);
            // Exercise click-path movement using the same pathfinder and world update as play.
            set_region();
            const auto &n = camp::Regions[0].sites[1];
            walk_to(int(n.x * Tile - camera_x), int(n.y * Tile - camera_y + Top));
            for (int i = 0; i < 2000 && screen == Explore; ++i)
                world(1. / 60.);
            check(screen == Dialogue && state.cleared[1], "walk reaches and interacts with Nara");
            while (screen == Dialogue)
                choose(110);
            interact(2);
            while (screen == Dialogue)
                choose(110);
            check(screen == Battle, "wild encounter starts");
            for (int i = 0; i < 2000 && screen == Battle; ++i) {
                if (duel.bodies[0].hp < Roster[duel.bodies[0].species].hp / 2)
                    choose(702);
                advance_battle();
            }
            check(screen == Result, "real engine encounter resolves");
            check(camp::validate(state), "valid postcombat campaign");
            choose(720);
            while (screen == Dialogue)
                choose(110);
            check(screen == Explore, "return from result");
            choose(100);
            choose(104);
            check(screen == Explore, "party returns to road");
            choose(101);
            choose(104);
            check(screen == Explore, "atlas returns");
            choose(103);
            choose(104);
            check(screen == Explore, "help returns");
            // A restored-region fixture exercises the actual expedition controller.
            for (int n : {1, 3, 4, 6, 9, 11, 12, 16})
                state.cleared[n] = 1;
            interact(18);
            while (screen == Dialogue)
                choose(110);
            check(screen == Lantern && state.walk_region == 0, "walk begins after keeper");
            choose(100);
            choose(104);
            check(screen == Lantern, "party returns to expedition fork");
            screen = Result;
            won = true;
            match.walk = true;
            choose(104);
            check(screen == Lantern, "result back preserves expedition commitment");
            choose(901);
            check(screen == Battle && match.walk && camp::trained_opponent(state, match),
                  "restless path uses a learned opponent");
            choose(703);
            check(screen == Result && state.walk_region == -1, "retreat exits expedition");
            choose(720);
            check(screen == Explore, "retreat returns to road");
            persist();
            camp::State copy;
            std::string error;
            check(camp::load(copy, options.save_path, error), "save reload");
            check(camp::serialize(copy) == camp::serialize(state), "save equality");
            std::error_code ignored;
            std::filesystem::remove_all(options.save_path.parent_path(), ignored);
            std::cout << checks
                      << " campaign controller checks passed, including a real learned-pilot "
                         "encounter.\n";
            return 0;
        } catch (const std::exception &e) {
            std::cerr << "Campaign controller test: " << e.what() << "\n";
            return 2;
        }
    }
    int run() {
        if (!art_ready) {
            std::cerr
                << "Campaign artwork is incomplete. Rebuild or supply the full assets directory.\n";
            return 2;
        }
        if (options.test)
            return controller_test();
        uint64_t last = SDL_GetPerformanceCounter();
        int frames = 0;
        while (running) {
            uint64_t now = SDL_GetPerformanceCounter();
            double dt = std::min(.1, double(now - last) / SDL_GetPerformanceFrequency());
            last = now;
            ++present;
            if (toast_age > 0)
                --toast_age;
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                    break;
                }
                if (!preview && e.type == SDL_WINDOWEVENT &&
                    e.window.event == SDL_WINDOWEVENT_FOCUS_LOST && screen == Battle)
                    paused = true;
                if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                    auto k = e.key.keysym.sym;
                    if (k == SDLK_F8) {
                        sound.enabled.store(!sound.enabled.load());
                        continue;
                    }
                    if (k == SDLK_F5 && has_save && screen != Battle && screen != Title &&
                        screen != Starters) {
                        persist(true);
                        continue;
                    }
                    if (screen == Battle) {
                        if (k == SDLK_ESCAPE)
                            choose(703);
                        else if (k == SDLK_m)
                            choose(700);
                        else if (k == SDLK_p)
                            choose(701);
                        else if (k == SDLK_r)
                            choose(702);
                        else if (k == SDLK_h)
                            show_hitboxes = !show_hitboxes;
                        else if (k >= SDLK_1 && k <= SDLK_4)
                            choose(710 + int(k - SDLK_1));
                        else if (k == SDLK_SPACE)
                            choose(714);
                        else if (k == SDLK_z)
                            wind_power = std::max(25, wind_power - 25);
                        else if (k == SDLK_x)
                            wind_power = std::min(100, wind_power + 25);
                    } else if (screen == Dialogue) {
                        if (k == SDLK_RETURN || k == SDLK_e || k == SDLK_SPACE)
                            choose(110);
                        else if (k == SDLK_ESCAPE && dialog_next == 1)
                            choose(104);
                    } else if (screen == Explore) {
                        if (k == SDLK_e || k == SDLK_RETURN) {
                            int near = near_site();
                            if (near >= 0)
                                interact(near);
                        } else if (k == SDLK_b)
                            choose(100);
                        else if (k == SDLK_TAB)
                            choose(101);
                        else if (k == SDLK_j)
                            choose(102);
                        else if (k == SDLK_ESCAPE || k == SDLK_F1 || k == SDLK_QUESTION)
                            choose(103);
                    } else if (k == SDLK_ESCAPE && screen == Notes)
                        screen = Party;
                    else if (k == SDLK_ESCAPE && screen == Studio)
                        screen = Title;
                    else if (k == SDLK_ESCAPE && screen != Title && screen != Lantern)
                        choose(104);
                    else if (screen == Result && (k == SDLK_RETURN || k == SDLK_SPACE))
                        choose(720);
                }
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    // SDL mouse events already use logical coordinates with RenderSetLogicalSize.
                    float x = float(e.button.x), y = float(e.button.y);
                    int picked = -1;
                    for (auto &b : buttons)
                        if (x >= b.box.x && x < b.box.x + b.box.w && y >= b.box.y &&
                            y < b.box.y + b.box.h) {
                            picked = b.id;
                            break;
                        }
                    if (picked >= 0)
                        choose(picked);
                    else if (screen == Explore && y >= Top && y < Bottom)
                        walk_to(int(x), int(y));
                }
            }
            if ((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) &&
                !(screen == Battle && paused) && screen != Title && screen != Starters &&
                screen != Credits && screen != Studio) {
                played_clock += dt;
                while (played_clock >= 1) {
                    state.play_seconds = std::min(100000000, state.play_seconds + 1);
                    played_clock -= 1;
                }
            }
            if (screen == Explore) {
                world(dt);
                save_clock += dt;
                if (save_clock > 15) {
                    persist();
                    save_clock = 0;
                }
            }
            if (screen == Battle && !paused && !preview) {
                accumulator += dt;
                while (accumulator >= .1 && screen == Battle) {
                    advance_battle();
                    accumulator -= .1;
                }
            }
            buttons.clear();
            switch (screen) {
            case Title:
                title();
                break;
            case Starters:
                starters();
                break;
            case Explore:
                draw_world();
                break;
            case Dialogue:
                draw_dialog();
                break;
            case Party:
                party();
                break;
            case Atlas:
                atlas();
                break;
            case Journal:
                journal();
                break;
            case Battle:
                battle();
                break;
            case Result:
                result();
                break;
            case Bells:
                draw_bells();
                break;
            case FinalChoice:
                final_choice();
                break;
            case Credits:
                credits();
                break;
            case Lantern:
                lantern();
                break;
            case Studio:
                studio();
                break;
            case Inn:
                inn();
                break;
            case Notes:
                notes();
                break;
            case Help:
                help();
                break;
            }
            if (!message.empty() && screen != Title && toast_age > 0) {
                frame(330, 6, 440, 30, pale);
                label(342, 17, message.substr(0, 70), ink, 1);
            }
            if (options.frames && ++frames >= options.frames) {
                std::error_code ec;
                std::filesystem::create_directories(options.captures, ec);
                int ww, hh;
                SDL_GetRendererOutputSize(r, &ww, &hh);
                auto *surface =
                    SDL_CreateRGBSurfaceWithFormat(0, ww, hh, 32, SDL_PIXELFORMAT_ARGB8888);
                if (surface) {
                    SDL_RenderReadPixels(r, nullptr, SDL_PIXELFORMAT_ARGB8888, surface->pixels,
                                         surface->pitch);
                    SDL_SaveBMP(surface, (options.captures + "/journey.bmp").c_str());
                    SDL_FreeSurface(surface);
                }
                std::cout << "Campaign render / region " << state.region << " / screen " << screen
                          << "\n";
                running = false;
            }
            SDL_RenderPresent(r);
            if (options.frames)
                SDL_Delay(1);
        }
        if (has_save && screen != Battle && screen != Title && screen != Starters)
            persist();
        return 0;
    }
};
int run(SDL_Window *window, Brain &brain, const std::filesystem::path &art,
        const Options &options) {
    Game game(window, brain, art, options);
    return game.run();
}
} // namespace journey

#pragma once
// Render-only skin: receives a const World and emits UI commands through shared buttons.
// Art animation uses simulation time; no RNG, movement, or ability logic lives here.
namespace tinikami {
#include "spirit_rects.hpp"
constexpr SDL_Color ink{39, 43, 48, 255}, paper{239, 231, 202, 255}, pale{250, 244, 220, 255},
    moss{71, 101, 78, 255}, jade{37, 122, 111, 255}, vermilion{184, 79, 59, 255},
    gold{195, 145, 65, 255}, muted{126, 129, 105, 255}, night{33, 54, 51, 255};
struct Atlas {
    SDL_Texture *texture = nullptr, *silhouette = nullptr;
    bool spirit_crops = false;
    int width = 0, height = 0, cols = 4, rows = 4;
    bool load(const std::filesystem::path &file, int c, int row, bool crops = false,
              bool subdued = false, bool landscape = false) {
        spirit_crops = crops;
        std::ifstream f(file, std::ios::binary);
        char magic[4];
        if (!f.read(magic, 4) || std::string(magic, 4) != "TINI")
            return false;
        auto read = [&]() {
            unsigned char b[4]{};
            f.read(reinterpret_cast<char *>(b), 4);
            return int(uint32_t(b[0]) | uint32_t(b[1]) << 8 | uint32_t(b[2]) << 16 |
                       uint32_t(b[3]) << 24);
        };
        width = read();
        height = read();
        cols = c;
        rows = row;
        if (!f || width < cols || height < rows || width > 4096 || height > 4096)
            return false;
        std::vector<uint8_t> data(size_t(width) * height * 4);
        if (!f.read(reinterpret_cast<char *>(data.data()), data.size()))
            return false;
        if (subdued) {
            // Renderer-only color grading reserves the full value/chroma range for actors and VFX.
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < width; ++x) {
                    auto *p = data.data() + (size_t(y) * width + x) * 4;
                    int lum = (54 * p[0] + 183 * p[1] + 19 * p[2]) / 256;
                    bool prop = y >= height * 3 / 4;
                    int value = prop ? 25 + lum * 55 / 100 : 106 + (lum - 128) * 24 / 100;
                    for (int channel = 0; channel < 3; ++channel)
                        p[channel] = uint8_t(
                            std::clamp(value + (int(p[channel]) - lum) * (prop ? 65 : 55) / 100,
                                       prop ? 25 : 45, prop ? 180 : 150));
                }
        }
        if (landscape) {
            // Gentle environment grading; preserve actor/VFX saturation and asset originals.
            for (int y = 0; y < height; ++y)
                for (int x = 0; x < width; ++x) {
                    auto *p = data.data() + (size_t(y) * width + x) * 4;
                    int lum = (54 * p[0] + 183 * p[1] + 19 * p[2]) / 256;
                    int saturation = (x * cols / width) == 2 ? 55 : 80;
                    for (int c = 0; c < 3; ++c)
                        p[c] = uint8_t(std::clamp(
                            lum * 92 / 100 + (int(p[c]) - lum) * saturation / 100, 0, 255));
                }
        }
        texture =
            SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
        if (!texture)
            return false;
        SDL_UpdateTexture(texture, nullptr, data.data(), width * 4);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        if (!subdued) {
            for (size_t i = 0; i < data.size(); i += 4)
                data[i] = data[i + 1] = data[i + 2] = 255;
            silhouette = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
                                           width, height);
            if (!silhouette)
                return false;
            SDL_UpdateTexture(silhouette, nullptr, data.data(), width * 4);
            SDL_SetTextureBlendMode(silhouette, SDL_BLENDMODE_BLEND);
        }
        return true;
    }
    SDL_Rect cell(int id) const {
        if (spirit_crops)
            return SpiritRects[id];
        int x = id % cols, y = id / cols;
        return {x * width / cols, y * height / rows, (x + 1) * width / cols - x * width / cols,
                (y + 1) * height / rows - y * height / rows};
    }
    void draw(int id, int x, int y, int w, int h, double angle = 0, bool flip = false,
              int alpha = 255, bool mask = false, SDL_Color tint = pale) const {
        if (!texture)
            return;
        auto src = cell(id);
        if (spirit_crops) {
            int fit_w = std::min(w, h * src.w / src.h), fit_h = std::min(h, w * src.h / src.w);
            x += (w - fit_w) / 2;
            y += h - fit_h;
            w = fit_w;
            h = fit_h;
        }
        SDL_Rect dst{x, y, w, h};
        auto *tex = mask ? silhouette : texture;
        if (!tex)
            return;
        SDL_SetTextureColorMod(tex, mask ? tint.r : 255, mask ? tint.g : 255, mask ? tint.b : 255);
        SDL_SetTextureAlphaMod(tex, uint8_t(std::clamp(alpha, 0, 255)));
        SDL_RenderCopyEx(r, tex, &src, &dst, angle, nullptr,
                         flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
    void free() {
        if (texture)
            SDL_DestroyTexture(texture);
        texture = nullptr;
        if (silhouette)
            SDL_DestroyTexture(silhouette);
        silhouette = nullptr;
    }
};
Atlas spirits, terrain, effects;
std::array<Atlas, SpeciesCount> animations;
std::array<bool, SpeciesCount> animated{};
int sprite_size(int species) {
    return std::clamp(44 + (Roster[species].radius - 340) * 72 / 320, 40, 120);
}
int orientation(Vec facing) {
    if (std::abs(facing.x) > std::abs(facing.y))
        return facing.x > 0 ? 1 : 3;
    return facing.y >= 0 ? 0 : 2;
}
void spirit(int species, int x, int ground_y, int size, Vec facing, int pose = 0, int alpha = 255,
            bool mask = false, SDL_Color tint = pale) {
    if (animated[species]) {
        // Imported sheets share a normalized foot pivot at 88/96 cell height.
        animations[species].draw(orientation(facing) * 8 + std::clamp(pose, 0, 7), x - size / 2,
                                 ground_y - size * 88 / 96, size, size, 0, false, alpha, mask,
                                 tint);
    } else
        spirits.draw(species, x - size / 2, ground_y - size * 3 / 4, size, size, 0, facing.x < 0,
                     alpha, mask, tint);
}
bool load(const std::filesystem::path &dir) {
    for (int i = 0; i < SpeciesCount; ++i)
        animated[i] = animations[i].load(dir / "animations" / (std::to_string(i) + ".rgba"), 8, 4);
    return spirits.load(dir / "spirits.rgba", 8, 5, true) &&
           terrain.load(dir / "environment-v2.rgba", 8, 4, false, true) &&
           effects.load(dir / "effects.rgba", 4, 4);
}
void free() {
    for (auto &a : animations)
        a.free();
    spirits.free();
    terrain.free();
    effects.free();
}
void frame(int x, int y, int w, int h, SDL_Color fill = paper) {
    rect(x, y, w, h, ink);
    rect(x + 3, y + 3, w - 6, h - 6, fill);
    line(x + 5, y + 5, x + w - 6, y + 5, pale);
    rect(x, y, 3, 3, fill);
    rect(x + w - 3, y, 3, 3, fill);
    rect(x, y + h - 3, 3, 3, fill);
    rect(x + w - 3, y + h - 3, 3, 3, fill);
}
void tag(int id, int x, int y, int w, const std::string &s, bool selected = false) {
    frame(x, y, w, 28, selected ? moss : paper);
    label(x + 9, y + 10, s, selected ? pale : ink, 1);
    buttons.push_back({{x, y, w, 28}, s, id});
}
void bar(int x, int y, int w, int value, int maximum, SDL_Color c, bool ticks = false) {
    rect(x, y, w, 10, ink);
    rect(x + 2, y + 2, w - 4, 6, muted);
    rect(x + 2, y + 2, (w - 4) * std::clamp(value, 0, maximum) / maximum, 6, c);
    if (ticks)
        for (int i = 1; i < 5; ++i)
            rect(x + w * i / 5, y + 2, 2, 6, paper);
}
std::string seconds(int ticks) {
    return std::to_string(ticks / 30) + "." + std::to_string(ticks % 30 / 3);
}
int effect_id(const Move &m) {
    if (m.heal || m.cleanse)
        return 14;
    if (m.wind_strength)
        return 7;
    if (m.element == Heat)
        return 1;
    if (m.element == Chill)
        return 3;
    if (m.element == Shock)
        return 2;
    if (m.element == Splash)
        return 4;
    if (m.poison || m.wound)
        return 5;
    if (m.silence || m.drain || m.surface == Oil)
        return 6;
    if (m.kind == Evade || m.kind == Blink)
        return 15;
    if (m.shield || m.guard)
        return 11;
    return m.kind == Melee || m.kind == Lunge ? 0 : 8;
}
SDL_Color effect_color(const Move &m) {
    int id = effect_id(m);
    if (id == 1)
        return {255, 151, 84, 255};
    if (id == 2 || id == 8 || id == 0)
        return {255, 232, 156, 255};
    if (id == 3 || id == 4 || id == 11)
        return {151, 225, 255, 255};
    if (id == 6)
        return {222, 165, 255, 255};
    return {154, 242, 189, 255};
}
void fx(int id, Vec p, int size, double angle = 0, int alpha = 255, bool active = true) {
    int x = sx(p.x) - size / 2, y = sy(p.y) - size / 2;
    if (active && alpha >= 175) {
        // Crisp one-pixel light rim, rather than a broad bloom that conceals trajectories.
        for (auto offset : {Vec{-1, 0}, Vec{1, 0}, Vec{0, -1}, Vec{0, 1}})
            effects.draw(id, x + offset.x, y + offset.y, size, size, angle, false, alpha * 3 / 4,
                         true, {250, 244, 209, 255});
    }
    effects.draw(id, x, y, size, size, angle, false, alpha);
}
#include "garden.hpp"
void warning(const World &w, int owner, bool debug) {
    const auto &b = w.bodies[owner];
    if (b.move < 0 || phase(b) == Recovery)
        return;
    const auto &m = Moves[b.move];
    bool starting = phase(b) == Startup;
    SDL_Color c = starting ? SDL_Color{236, 193, 100, 255} : effect_color(m);
    int alpha = starting ? 24 : 52;
    auto disk = [&](Vec p, int radius) {
        worldcircle(p, radius, {c.r, c.g, c.b, uint8_t(alpha)}, true);
        worldcircle(p, radius + 35, {27, 36, 40, 200});
        worldcircle(p, radius, c);
    };
    Vec end = b.pos + scale(b.locked, m.range);
    double angle = std::atan2(double(b.locked.y), double(b.locked.x)) * 180 / 3.14159265;
    if (m.kind == Melee || m.kind == Lunge) {
        Vec center = b.pos + scale(b.locked, m.range / 2);
        disk(center, m.radius);
        if (m.kind == Lunge) {
            int remaining = starting ? m.active : std::max(0, m.startup + m.active - b.age);
            corridor(center, center + scale(b.locked, m.speed * remaining), m.radius, c);
        }
        if (!starting)
            fx(effect_id(m), center, std::max(36, m.radius * S * 2 / Q), angle);
    } else if (m.kind == Bolt || m.kind == Beam) {
        for (int shot = 0; shot < m.shots; ++shot) {
            Vec dir = unit(b.locked + scale(Vec{-b.locked.y, b.locked.x},
                                            (2 * shot - (m.shots - 1)) * m.spread, 2 * Q));
            Vec begin = b.pos + scale(dir, m.min_range), finish = b.pos + scale(dir, m.range);
            corridor(begin, finish, m.radius, c);
            if (!starting && m.kind == Beam) {
                for (int n = 1; n <= 8; ++n)
                    fx(effect_id(m), begin + scale(finish - begin, n, 8),
                       std::max(24, m.radius * S * 2 / Q), angle, 200);
            }
        }
    } else if (m.kind == Nova) {
        disk(b.pos, m.radius);
        if (starting) {
            int segments = b.age * 96 / std::max(1, m.startup);
            for (int n = 0; n < segments; ++n) {
                double a = n * 6.283185307 / 96 - 1.570796327;
                double z = (n + 1) * 6.283185307 / 96 - 1.570796327;
                for (int edge : {55, 100})
                    worldline(b.pos + Vec{int(std::cos(a) * (m.radius + edge)), int(std::sin(a) * (m.radius + edge))},
                              b.pos + Vec{int(std::cos(z) * (m.radius + edge)), int(std::sin(z) * (m.radius + edge))},
                              {255, 227, 145, 255});
            }
        }
        if (m.min_range)
            worldcircle(b.pos, m.min_range, ink);
        if (!starting)
            fx(effect_id(m), b.pos, m.radius * S * 2 / Q, w.tick * 5 % 360, 180);
    } else if (m.kind == Field || m.kind == Trap || m.kind == Turret) {
        end = target_point(w, owner, b.move);
        disk(end, m.radius);
        fx(8, end, std::max(32, m.radius * S / Q), 0, starting ? 120 : 220);
    } else if (m.kind == Evade || m.kind == Blink) {
        int distance =
            m.kind == Evade ? m.speed * Roster[b.species].dodge_speed / 100 * m.active : m.range;
        if (debug || starting)
            corridor(b.pos, b.pos + scale(b.locked, distance), b.radius, jade);
        if (!starting)
            fx(15, b.pos - scale(b.locked, 400), 60, angle, 160);
    } else if (!starting)
        fx(effect_id(m), b.pos, 70, 0, 190);
    if (m.wind_strength)
        arrow(b.pos, b.locked, 2200, c);
    if (m.surface && starting) {
        Vec place = m.kind == Field || m.kind == Trap || m.kind == Turret ? end
                    : m.kind == Bolt ? b.pos + scale(b.locked, m.range)
                                     : b.pos;
        worldcircle(place, m.surface_radius, c);
    }
}
struct View {
    bool paused, manual, playback, catalog, hitboxes;
    int catalog_target, catalog_page, weather, speed, wind_power;
    uint32_t seed;
    std::string note;
    bool learned_a, learned_b, brain_ready;
    std::array<int, 2> personality_ids;
    std::array<bool, 2> baseline;
    int finish_age = 0;
    bool padded = false;
};
void draw(const World &w, const View &v) {
    struct CameraScope {
        int x = AX, y = AY, scale = S;
        ~CameraScope() { AX = x; AY = y; S = scale; }
    } camera;
    if (v.padded) { AX = JourneyAX; AY = JourneyAY; S = JourneyScale; }
    rect(0, 0, 1100, 780, night);
    frame(10, 10, 1080, 760);
    label(28, 25, "TINIKAMI", ink, 4);
    label(30, 66, "SMALL SPIRITS. WILD POSSIBILITIES.", moss, 1);
    tag(17, 282, 53, 94, "NEW SEED");
    tag(10, 382, 53, 94, "SPEED " + num(v.speed) + "X");
    tag(30, 482, 53, 154, "H / HITBOXES", v.hitboxes);
    tag(29, 858, 24, 208, "F2 / SWITCH TO WORKBENCH");
    tag(31, 646, 53, 128,
        v.manual        ? "A: HUMAN [M]"
        : v.baseline[0] ? "A: BASELINE [B]"
        : v.learned_a   ? "A: SPIRIT [B]"
                        : "A: SCRIPT [B]",
        v.learned_a && !v.manual);
    tag(32, 782, 53, 128,
        v.baseline[1] ? "B: BASELINE [V]"
        : v.learned_b ? "B: SPIRIT [V]"
                      : "B: SCRIPT [V]",
        v.learned_b);
    label(934, 68, "ALPHA 0.12", moss, 1);
    tag(1, 24, 93, 88, v.paused ? "RESUME [P]" : "PAUSE [P]", v.paused);
    tag(3, 118, 93, 82, "RESET [R]");
    tag(4, 206, 93, 118, v.manual ? "YOU + SPIRIT" : "WATCH SPIRITS", v.manual);
    tag(24, 330, 93, 100, "SPIRIT BOOK");
    tag(28, 436, 93, 132, arena_name(w.arena));
    tag(5, 574, 93, 146, v.weather == 0 ? "CLEAR SKY" : v.weather == 1 ? "BREEZY" : "SOFT RAIN");
    label(28, 135, "GARDEN " + seconds(w.tick) + "S  /  SEED " + num(v.seed), moss, 1);
    label(460, 135,
          v.playback ? "RECORDED REPLAY"
          : v.manual ? "WASD / MOUSE / 1-4 / SPACE"
                     : "M TO TAKE CONTROL",
          moss, 1);
    frame(20, 154, 704, 530, ink);
    SDL_Rect clip{24, 158, 696, 522};
    SDL_RenderSetClipRect(r, &clip);
    if (v.padded) rect(24, 158, 696, 522, Gardens[w.arena].floor);
    background(w);
    if (v.padded) {
        SDL_Rect bounds{AX, AY, 24 * S, 18 * S};
        color({167, 169, 141, 180});
        SDL_RenderDrawRect(r, &bounds);
    }
    for (const auto &g : w.surfaces)
        if (g.life)
            ground(g, w.tick, v.hitboxes);
    Vec wind = wind_vector(w);
    if (length(wind))
        for (int gy = 1; gy < 18; gy += 5)
            for (int gx = 1; gx < 24; gx += 6) {
                Vec center = Vec{gx * Q, gy * Q} + scale(wind, w.tick % 30, 1);
                arrow(center, wind, 350 + length(wind) * 22, {186, 207, 196, 85});
            }
    if (w.rain)
        for (int n = 0; n < 26; ++n) {
            int x = AX + (n * 83 + w.tick * 2) % 690, y = AY + (n * 71 + w.tick * 4) % 515;
            line(x, y, x + 3, y + 8, {152, 179, 184, 90});
        }
    if (w.objective) {
        worldcircle({12 * Q, 9 * Q}, 2500, {239, 231, 202, 110});
        terrain.draw(27, sx(12 * Q) - 26, sy(9 * Q) - 25, 52, 52, 0, false, 220);
    }
    if (w.vane_enabled) {
        worldcircle(w.vane_pos, 950, {239, 231, 202, 130});
        terrain.draw(26, sx(w.vane_pos.x) - 30, sy(w.vane_pos.y) - 39, 60, 60);
        arrow(w.vane_pos + Vec{0, 650}, length(wind) ? wind : Vec{Q, 0}, 700, pale);
        if (w.vane_cooldown)
            label(sx(w.vane_pos.x) - 9, sy(w.vane_pos.y) + 22, seconds(w.vane_cooldown), pale, 1);
        if (w.vane_capture[0] || w.vane_capture[1])
            bar(sx(w.vane_pos.x) - 24, sy(w.vane_pos.y) + 30, 48,
                std::max(w.vane_capture[0], w.vane_capture[1]), 45, gold);
    }
    for (const auto &z : w.zones)
        if (z.life) {
            const auto &m = Moves[z.move];
            auto c = effect_color(m);
            worldcircle(z.pos, m.radius, {c.r, c.g, c.b, 55}, true);
            worldcircle(z.pos, m.radius, z.owner ? vermilion : jade);
            int id = m.kind == Turret ? 9 : m.kind == Trap ? 10 : m.heal ? 14 : effect_id(m);
            fx(id, z.pos,
               m.kind == Turret ? 46
               : m.kind == Trap ? 36
                                : std::clamp(m.radius * S / Q, 30, 72),
               m.kind == Field ? (w.tick / 3 % 6 - 3) : 0, 185);
            if (z.hp)
                bar(sx(z.pos.x) - 18, sy(z.pos.y) - 30, 36, z.hp, 45, moss);
        }
    for (const auto &o : w.obstacles)
        if (o.radius) {
            int size = o.radius * S * 2 / Q + 16;
            terrain.draw(Gardens[w.arena].obstacle, sx(o.pos.x) - size / 2,
                         sy(o.pos.y) - size * 3 / 4, size, size);
            worldcircle(o.pos, o.radius, {25, 35, 39, 145});
            if (v.hitboxes)
                worldcircle(o.pos, o.radius, pale);
        }
    for (int i = 0; i < 2; ++i)
        warning(w, i, v.hitboxes);
    // Back-to-front body order is presentation only.
    for (int n = 0; n < 2; ++n) {
        int i = w.bodies[0].pos.y <= w.bodies[1].pos.y ? n : 1 - n;
        const auto &b = w.bodies[i];
        auto team = i ? vermilion : jade;
        int size = sprite_size(b.species);
        int pose = length(b.vel) > 15 ? 1 + (w.tick / 5) % 2 : 0;
        if (b.move >= 0)
            pose = phase(b) == Startup ? 3 : phase(b) == Active ? 4 : 0;
        int hurt_age = 1000, faint_age = 1000;
        for (int j = 0; j < w.event_count; ++j) {
            const auto &ev = w.events[(w.event_head - 1 - j + HistoryCount) % HistoryCount];
            if (ev.target != i)
                continue;
            if (ev.kind == Hit)
                hurt_age = std::min(hurt_age, w.tick - ev.tick);
            if (ev.kind == Knockout)
                faint_age = std::min(faint_age, w.tick - ev.tick);
        }
        if (hurt_age < 6)
            pose = 5;
        if (!b.hp)
            pose = faint_age + v.finish_age < 12 ? 6 : 7;
        int x = sx(b.pos.x), y = sy(b.pos.y);
        worldcircle(b.pos, b.radius, {23, 40, 43, 90}, true);
        worldcircle(b.pos, b.radius + 75, team);
        for (auto offset : {Vec{-1, 0}, Vec{1, 0}, Vec{0, -1}, Vec{0, 1}})
            spirit(b.species, x + offset.x, y + offset.y, size, b.aim, pose, b.hp ? 200 : 140, true,
                   {28, 36, 43, 255});
        spirit(b.species, x, y, size, b.aim, pose, b.hp ? 255 : 170);
        Vec nose = b.pos + scale(b.aim, b.radius + 480);
        Vec side = scale(Vec{-b.aim.y, b.aim.x}, 180);
        worldline(nose - scale(b.aim, 230) + side, nose, team);
        worldline(nose - scale(b.aim, 230) - side, nose, team);
        if (v.hitboxes) {
            worldcircle(b.pos, b.radius, pale);
            arrow(b.pos, b.aim, 1200, pale);
        }
        if (b.shield || b.guard) {
            arc(b.pos, b.aim, b.radius + 270, std::acos(1. / 3.), pale);
            fx(11, b.pos, size + 12, 0, 65);
        }
        if (b.burn)
            fx(1, b.pos + Vec{350, -250}, 25, -90, 210);
        if (b.poison)
            fx(6, b.pos + Vec{-350, -250}, 21, -90, 190);
        if (b.root)
            worldcircle(b.pos, b.radius + 180, jade);
        bar(x - 25, y - size * 88 / 96 - 10, 50, b.hp, Roster[b.species].hp, moss);
        if (footwork_load(b) >= 70 && b.hp) {
            line(x - 14, y + 15, x + 14, y + 15, gold);
            label(x - 21, y + 19, "STRAFE", gold, 1);
        }
        if (b.move >= 0) {
            auto ph = phase(b);
            const auto &m = Moves[b.move];
            if (ph == Startup)
                bar(x - 25, y + 22, 50, b.age, m.startup, gold);
            std::string name = ph == Startup ? m.name : ph == Recovery ? "RECOVER" : "";
            if (!name.empty()) {
                int tw = int(name.size()) * 6 + 8;
                int tx = std::clamp(x - tw / 2, clip.x, clip.x + clip.w - tw);
                int ty = std::min(y + 36, clip.y + clip.h - 14);
                rect(tx, ty, tw, 13, night);
                label(tx + 4, ty + 3, name, ph == Startup ? paper : muted, 1);
            }
        }
    }
    for (const auto &p : w.projectiles)
        if (p.life) {
            const auto &m = Moves[p.move];
            int size = std::clamp(m.radius * S * 2 / Q + 20, 22, 60);
            double angle = std::atan2(double(p.vel.y), double(p.vel.x)) * 180 / 3.14159265;
            fx(effect_id(m), p.pos - scale(unit(p.vel), 400), size, angle, 75);
            fx(effect_id(m), p.pos, size, angle);
            if (v.hitboxes)
                worldcircle(p.pos, m.kind == Turret ? 160 : m.radius, pale);
        }
    for (int j = 0; j < std::min(16, w.event_count); ++j) {
        const auto &ev = w.events[(w.event_head - 1 - j + HistoryCount) % HistoryCount];
        int age = w.tick - ev.tick;
        if (age < 0 || age > 15 || (ev.kind != Hit && ev.kind != Healed && ev.kind != Parried))
            continue;
        fx(ev.kind == Healed    ? 14
           : ev.kind == Parried ? 11
                                : 12,
           ev.pos, 22 + age * 2, 0, 220 - age * 12);
        if (ev.amount)
            label(sx(ev.pos.x) - 6, sy(ev.pos.y) - 36 - age * 2,
                  (ev.kind == Healed ? "+" : "-") + num(ev.amount), pale, 2);
    }
    if (w.tick >= 1800)
        worldcircle({12 * Q, 9 * Q}, 12 * Q - (w.tick - 1800) * 7 * Q / 900, vermilion);
    if (v.manual && !v.catalog) {
        int mx, my;
        float x, y;
        SDL_GetMouseState(&mx, &my);
        SDL_RenderWindowToLogical(r, mx, my, &x, &y);
        if (x >= AX && x < AX + 24 * S && y >= AY && y < AY + 18 * S) {
            line(int(x) - 7, int(y), int(x) - 3, int(y), pale);
            line(int(x) + 3, int(y), int(x) + 7, int(y), pale);
            line(int(x), int(y) - 7, int(x), int(y) - 3, pale);
            line(int(x), int(y) + 3, int(x), int(y) + 7, pale);
        }
    }
    SDL_RenderSetClipRect(r, nullptr);
    if (v.paused || w.terminal || w.truncated) {
        frame(260, 164, 224, 28, night);
        label(274, 174,
              w.terminal || w.truncated
                  ? (w.winner < 0 ? "DRAW / R TO REPLAY"
                                  : std::string(w.winner ? "B" : "A") + " WINS / R TO REPLAY")
                  : "PAUSED / P TO RESUME",
              pale, 1);
    }
    label(28, 699, "LOTUS CONTROL", moss, 1);
    bar(132, 696, 220, w.bodies[0].control, 600, jade);
    bar(390, 696, 220, w.bodies[1].control, 600, vermilion);
    label(628, 699, "H: GEOMETRY", moss, 1);
    label(28, 719,
          "WIND " + num(wind.x) + "," + num(wind.y) + "  CAST " + num(v.wind_power) + "% [Z/X]",
          moss, 1);
    label(408, 719, "F5 SAVE / F9 RESTORE / N STEP", moss, 1);
    for (int i = 0; i < 2; ++i) {
        const auto &b = w.bodies[i];
        const auto &s = Roster[b.species];
        int y = 93 + i * 133;
        frame(738, y, 330, 125);
        spirits.draw(b.species, 747, y + 22, 78, 78, 0, i == 1);
        tag(i ? 22 : 20, 749, y + 4, 28, "-");
        tag(i ? 23 : 21, 1029, y + 4, 28, "+");
        label(787, y + 14, std::string(i ? "B / " : "A / ") + s.name, i ? vermilion : jade, 1);
        label(833, y + 38, "VITALITY " + num(b.hp) + "/" + num(s.hp), ink, 1);
        bar(833, y + 52, 218, b.hp, s.hp, moss);
        label(833, y + 70, "ENERGY " + num(b.energy / 10) + "/100", jade, 1);
        bar(833, y + 84, 218, b.energy, 1000, jade, true);
        std::string state = energy_regen(b) ? "+" + num(energy_regen(b) * 3) + "/S RECHARGING"

                            : b.move >= 0            ? "CASTING / REGEN PAUSED"
                                                     : "BREATH " + seconds(b.energy_delay) + "S";
        label(753, y + 108, state, moss, 1);
        bool active = (i ? v.learned_b : v.learned_a && !v.manual) && !v.baseline[i];
        tag(34 + i, 915, y + 96, 143,
            v.playback
                ? "RECORDED"
                : std::string(personality_name(v.personality_ids[i])) + (i ? " [K]" : " [J]"),
            active && !v.playback);
    }
    frame(738, 365, 330, 226);
    label(751, 380, "A / CHOOSE YOUR MOMENT", ink, 1);
    auto mask = action_mask(w, 0);
    const auto &b = w.bodies[0];
    for (int j = 0; j < 5; ++j) {
        const auto &m = move_for(b, j);
        int y = 399 + j * 36;
        rect(747, y, 312, 33, j % 2 ? paper : pale);
        effects.draw(effect_id(m), 750, y + 2, 28, 28);
        label(785, y + 5, (j == 4 ? "SPC " : num(j + 1) + " ") + m.name, mask[j + 1] ? ink : muted,
              1);
        label(1018, y + 5, num(m.cost / 10) + "E", jade, 1);
        std::string status = b.cooldown[j] ? "COOLDOWN " + seconds(b.cooldown[j]) + "S"
                             : b.energy < m.cost
                                 ? "NEED " + num((m.cost - b.energy + 9) / 10) + " ENERGY"
                             : b.move >= 0  ? "CAST IN PROGRESS"
                             : b.stun       ? "STUNNED"
                             : !mask[j + 1] ? "UNAVAILABLE"
                                            : "READY";
        label(785, y + 21, status, mask[j + 1] ? moss : muted, 1);
        label(968, y + 21, m.move_start || j == 4 ? "MOBILE" : "PLANTED", muted, 1);
        buttons.push_back({{747, y, 312, 33}, "MOVE", 50 + j});
    }
    frame(738, 598, 330, 132);
    label(751, 613, "GUIDE YOUR SPIRIT", ink, 1);
    tag(11, 750, 632, 95, "ATTACK", b.guidance == Attack);
    tag(12, 850, 632, 95, "RETREAT", b.guidance == Retreat);
    tag(13, 950, 632, 106, "CONSERVE", b.guidance == Conserve);
    tag(14, 750, 667, 95, "FREE", b.guidance == Free);
    tag(8, 850, 667, 95, "SAVE REPLAY");
    tag(9, 950, 667, 106, "LOAD REPLAY");
    label(752, 710, "ONE POOL / FOUR ARTS + DODGE", moss, 1);
    label(28, 746, v.note.substr(0, 145), moss, 1);
    if (v.catalog) {
        buttons.clear();
        frame(60, 80, 980, 660);
        label(82, 100, "THE SPIRIT BOOK", ink, 3);
        label(84, 137, "FORTY SPIRITS / FOUR ARTS EACH / ONE SHARED ENERGY POOL", moss, 1);
        tag(25, 82, 158, 175, v.catalog_target ? "CHOOSE OPPONENT / B" : "CHOOSE YOUR SPIRIT / A");
        tag(26, 662, 158, 100, "PREVIOUS");
        tag(27, 770, 158, 100, "NEXT");
        tag(24, 880, 158, 138, "CLOSE [TAB]");
        label(282, 169, "PAGE " + num(v.catalog_page + 1) + " / 4", moss, 1);
        for (int j = 0; j < 10; ++j) {
            int id = v.catalog_page * 10 + j, x = 82 + (j % 2) * 474, y = 202 + (j / 2) * 97;
            const auto &sp = Roster[id];
            frame(x, y, 461, 90, id == w.bodies[v.catalog_target].species ? pale : paper);
            spirits.draw(id, x + 8, y + 5, 80, 80);
            label(x + 100, y + 13, num(id + 1) + " " + sp.name, jade, 2);
            label(x + 100, y + 38, sp.role, moss, 1);
            label(x + 100, y + 57, "HP " + num(sp.hp) + " / ENERGY +" + num(sp.regen * 3) + "/S",
                  ink, 1);
            label(x + 100, y + 73,
                  "ARTS " + num(Moves[id * 4].cost / 10) + " / " +
                      num(Moves[id * 4 + 1].cost / 10) + " / " + num(Moves[id * 4 + 2].cost / 10) +
                      " / " + num(Moves[id * 4 + 3].cost / 10),
                  muted, 1);
            buttons.push_back({{x, y, 461, 90}, sp.name, 100 + id});
        }
        label(84, 712,
              "SELECT A SPIRIT TO BEGIN A FRESH ENCOUNTER. F2 RETURNS TO THE DEBUGGING SKIN.", moss,
              1);
    }
}
} // namespace tinikami

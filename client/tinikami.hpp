#pragma once
// Render-only skin: receives a const World and emits UI commands through shared buttons.
// Art animation uses simulation time; no RNG, movement, or ability logic lives here.
namespace tinikami {
#include "spirit_rects.hpp"
constexpr SDL_Color ink{39, 43, 48, 255}, paper{239, 231, 202, 255}, pale{250, 244, 220, 255},
    moss{71, 101, 78, 255}, jade{37, 122, 111, 255}, vermilion{184, 79, 59, 255},
    gold{195, 145, 65, 255}, muted{126, 129, 105, 255}, night{33, 54, 51, 255};
struct Atlas {
    SDL_Texture *texture = nullptr;
    int width = 0, height = 0, cols = 4, rows = 4;
    bool load(const std::filesystem::path &file, int c, int row) {
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
        texture =
            SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
        if (!texture)
            return false;
        SDL_UpdateTexture(texture, nullptr, data.data(), width * 4);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return true;
    }
    SDL_Rect cell(int id) const {
        if (cols == 8)
            return SpiritRects[id];
        int x = id % cols, y = id / cols;
        return {x * width / cols, y * height / rows, (x + 1) * width / cols - x * width / cols,
                (y + 1) * height / rows - y * height / rows};
    }
    void draw(int id, int x, int y, int w, int h, double angle = 0, bool flip = false,
              int alpha = 255) const {
        if (!texture)
            return;
        auto src = cell(id);
        if (cols == 8) {
            int fit_w = std::min(w, h * src.w / src.h), fit_h = std::min(h, w * src.h / src.w);
            x += (w - fit_w) / 2;
            y += h - fit_h;
            w = fit_w;
            h = fit_h;
        }
        SDL_Rect dst{x, y, w, h};
        SDL_SetTextureAlphaMod(texture, uint8_t(std::clamp(alpha, 0, 255)));
        SDL_RenderCopyEx(r, texture, &src, &dst, angle, nullptr,
                         flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
    void free() {
        if (texture)
            SDL_DestroyTexture(texture);
        texture = nullptr;
    }
};
Atlas spirits, terrain, effects;
bool load(const std::filesystem::path &dir) {
    return spirits.load(dir / "spirits.rgba", 8, 5) &&
           terrain.load(dir / "environment.rgba", 4, 4) && effects.load(dir / "effects.rgba", 4, 4);
}
void free() {
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
        return vermilion;
    if (id == 2 || id == 8 || id == 0)
        return gold;
    if (id == 3 || id == 4 || id == 11)
        return {74, 153, 180, 255};
    if (id == 6)
        return {135, 86, 167, 255};
    return jade;
}
void fx(int id, Vec p, int size, double angle = 0, int alpha = 255) {
    effects.draw(id, sx(p.x) - size / 2, sy(p.y) - size / 2, size, size, angle, false, alpha);
}
// Exact circular surface edge, tiled interior, in small blocks for a pixel contour.
void ground(const Surface &g, int tick) {
    static const int tile[] = {0, 3, 4, 7, 8, 9, 10, 5, 6};
    int rad = g.radius * S / Q, cx = sx(g.pos.x), cy = sy(g.pos.y);
    const auto src = terrain.cell(tile[g.kind]);
    SDL_SetTextureAlphaMod(terrain.texture, g.kind == Steam ? 150 : 225);
    for (int y = -rad; y < rad; y += 4) {
        int half = int(std::sqrt(double(std::max(0, rad * rad - y * y))));
        for (int x = -half; x < half; x += 4) {
            int u = ((cx + x) % 64 + 64) % 64, v = ((cy + y) % 64 + 64) % 64;
            SDL_Rect a{src.x + u * src.w / 64, src.y + v * src.h / 64, std::max(1, src.w / 16),
                       std::max(1, src.h / 16)};
            SDL_Rect b{cx + x, cy + y, std::min(4, half - x), 4};
            SDL_RenderCopy(r, terrain.texture, &a, &b);
        }
    }
    worldcircle(g.pos, g.radius, {33, 63, 59, 155});
    if ((g.kind == Water || g.kind == ChargedWater) && length(g.flow))
        for (int offset : {-1, 1}) {
            Vec p = g.pos + Vec{offset * g.radius / 3, -g.radius / 3};
            arrow(p + scale(g.flow, tick % 24, 1), g.flow, 470, pale);
        }
    if (g.kind == Fire) {
        fx(1, g.pos, 28 + (tick / 4 % 3) * 3, -90, 225);
        fx(1, g.pos + Vec{g.radius / 3, g.radius / 3}, 20, -90, 200);
    }
    if (g.kind == ChargedWater && tick % 24 < 9)
        fx(2, g.pos, std::min(60, rad), 15, 220);
    if (g.effect_timer || g.owner >= 0) {
        int life = g.effect_timer ? g.effect_timer : g.life;
        label(cx - 9, cy + rad - 10, seconds(life), pale, 1);
    }
}
void warning(const World &w, int owner, bool debug) {
    const auto &b = w.bodies[owner];
    if (b.move < 0 || phase(b) == Recovery)
        return;
    const auto &m = Moves[b.move];
    bool starting = phase(b) == Startup;
    SDL_Color c = starting ? gold : effect_color(m);
    int alpha = starting ? 35 : 65;
    auto disk = [&](Vec p, int radius) {
        worldcircle(p, radius, {c.r, c.g, c.b, uint8_t(alpha)}, true);
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
};
void draw(const World &w, const View &v) {
    rect(0, 0, 1100, 780, night);
    frame(10, 10, 1080, 760);
    label(28, 25, "TINIKAMI", ink, 4);
    label(30, 66, "SMALL SPIRITS. WILD POSSIBILITIES.", moss, 1);
    tag(17, 282, 53, 94, "NEW SEED");
    tag(10, 382, 53, 94, "SPEED " + num(v.speed) + "X");
    tag(30, 482, 53, 154, "H / HITBOXES", v.hitboxes);
    tag(29, 858, 24, 208, "F2 / SWITCH TO WORKBENCH");
    label(742, 68, "SPIRIT GARDEN   /   ALPHA 0.6", moss, 1);
    tag(1, 24, 93, 88, v.paused ? "RESUME [P]" : "PAUSE [P]", v.paused);
    tag(3, 118, 93, 82, "RESET [R]");
    tag(4, 206, 93, 118, v.manual ? "YOU + SPIRIT" : "WATCH SPIRITS", v.manual);
    tag(24, 330, 93, 100, "SPIRIT BOOK");
    tag(28, 436, 93, 132,
        w.arena == 0   ? "STONE GARDEN"
        : w.arena == 1 ? "MOSS GROVE"
                       : "OPEN MEADOW");
    tag(5, 574, 93, 146, v.weather == 0 ? "CLEAR SKY" : v.weather == 1 ? "BREEZY" : "SOFT RAIN");
    label(28, 135, "GARDEN " + seconds(w.tick) + "S  /  SEED " + num(v.seed), moss, 1);
    label(460, 135,
          v.playback ? "RECORDED REPLAY"
          : v.manual ? "WASD / MOUSE / 1-4 / SPACE"
                     : "M TO TAKE CONTROL",
          moss, 1);
    frame(20, 154, 704, 530, ink);
    SDL_Rect clip{AX, AY, 24 * S, 18 * S};
    SDL_RenderSetClipRect(r, &clip);
    for (int gy = 0; gy < 9; ++gy)
        for (int gx = 0; gx < 12; ++gx) {
            int tile = (gx == 5 || gx == 6 || gy == 4) ? 1 : ((gx * 7 + gy * 13) % 9 == 0 ? 11 : 0);
            terrain.draw(tile, AX + gx * 58, AY + gy * 58, 58, 58);
        }
    rect(AX, AY, 24 * S, 18 * S, {222, 229, 201, 28});
    for (const auto &g : w.surfaces)
        if (g.life)
            ground(g, w.tick);
    Vec wind = wind_vector(w);
    if (length(wind))
        for (int gy = 1; gy < 18; gy += 4)
            for (int gx = 1; gx < 24; gx += 4) {
                Vec center = Vec{gx * Q, gy * Q} + scale(wind, w.tick % 30, 1);
                arrow(center, wind, 350 + length(wind) * 22, {239, 244, 213, 135});
            }
    if (w.rain)
        for (int n = 0; n < 45; ++n) {
            int x = AX + (n * 83 + w.tick * 2) % 690, y = AY + (n * 71 + w.tick * 4) % 515;
            line(x, y, x + 3, y + 8, {182, 214, 206, 150});
        }
    worldcircle({12 * Q, 9 * Q}, 2500, {239, 231, 202, 110});
    terrain.draw(14, sx(12 * Q) - 26, sy(9 * Q) - 25, 52, 52, 0, false, 220);
    if (w.vane_enabled) {
        worldcircle(w.vane_pos, 950, {239, 231, 202, 130});
        terrain.draw(13, sx(w.vane_pos.x) - 30, sy(w.vane_pos.y) - 39, 60, 60);
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
            terrain.draw(w.arena == 1 ? 15 : 12, sx(o.pos.x) - size / 2, sy(o.pos.y) - size * 3 / 4,
                         size, size);
            if (v.hitboxes)
                worldcircle(o.pos, o.radius, ink);
        }
    for (int i = 0; i < 2; ++i)
        warning(w, i, v.hitboxes);
    // Back-to-front body order is presentation only.
    for (int n = 0; n < 2; ++n) {
        int i = w.bodies[0].pos.y <= w.bodies[1].pos.y ? n : 1 - n;
        const auto &b = w.bodies[i];
        auto team = i ? vermilion : jade;
        int size = std::clamp(54 + b.radius * 36 / Q, 60, 84);
        int bob = b.move >= 0 ? 0 : (w.tick / (length(b.vel) > 15 ? 3 : 9) + b.species) % 4 / 2 * 2;
        int x = sx(b.pos.x), y = sy(b.pos.y);
        worldcircle(b.pos, b.radius, {23, 40, 43, 90}, true);
        worldcircle(b.pos, b.radius + 75, team);
        spirits.draw(b.species, x - size / 2, y - size * 3 / 4 - bob, size, size, 0, b.aim.x < 0,
                     b.hp ? 255 : 80);
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
        bar(x - 25, y - size * 3 / 4 - 10, 50, b.hp, Roster[b.species].hp, moss);
        if (b.move >= 0) {
            auto ph = phase(b);
            const auto &m = Moves[b.move];
            if (ph == Startup)
                bar(x - 25, y + 22, 50, b.age, m.startup, gold);
            std::string name = ph == Startup ? m.name : ph == Recovery ? "RECOVER" : "";
            if (!name.empty()) {
                int tw = int(name.size()) * 6 + 8;
                rect(x - tw / 2, y + 36, tw, 13, night);
                label(x - tw / 2 + 4, y + 39, name, ph == Startup ? paper : muted, 1);
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
                            : b.move >= 0   ? "CASTING / REGEN PAUSED"
                                            : "BREATH " + seconds(b.energy_delay) + "S";
        label(753, y + 108, state, moss, 1);
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
                             : b.move >= 0  ? "COMMITTED"
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

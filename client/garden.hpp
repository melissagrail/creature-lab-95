#pragma once
// Within namespace tinikami. Pure presentation; no simulation RNG is advanced.
struct GardenStyle {
    SDL_Color floor;
    int base, path, obstacle;
};
constexpr GardenStyle Gardens[ArenaCount] = {
    {{97, 100, 105, 255}, 0, 12, 24}, {{98, 110, 94, 255}, 2, 13, 25},
    {{124, 111, 91, 255}, 4, 14, 29}, {{108, 99, 113, 255}, 6, 15, 28},
    {{99, 115, 128, 255}, 8, 12, 30}, {{120, 102, 89, 255}, 10, 14, 31}};
uint32_t garden_hash(uint32_t seed, int x, int y) {
    uint32_t h = seed ^ uint32_t(x) * 0x9e3779b9u ^ uint32_t(y) * 0x85ebca6bu;
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    return h ^ (h >> 16);
}
// A small alpha mesh fades every tile into the common floor; rotations cannot expose hard seams.
void soft_tile(int id, int x, int y, int size, uint32_t variant, int opacity) {
    auto src = terrain.cell(id);
    SDL_Vertex vertices[16]{};
    constexpr float edge[] = {0.f, .18f, .82f, 1.f};
    for (int gy = 0; gy < 4; ++gy)
        for (int gx = 0; gx < 4; ++gx) {
            float u = edge[gx], v = edge[gy];
            if (variant & 4)
                u = 1 - u;
            for (unsigned i = 0; i < (variant & 3); ++i) {
                float old = u;
                u = v;
                v = 1 - old;
            }
            auto &p = vertices[gy * 4 + gx];
            p.position = {float(x) + edge[gx] * size, float(y) + edge[gy] * size};
            p.color = {255, 255, 255, uint8_t(gx && gx < 3 && gy && gy < 3 ? opacity : 0)};
            p.tex_coord = {(src.x + 1 + u * (src.w - 2)) / terrain.width,
                           (src.y + 1 + v * (src.h - 2)) / terrain.height};
        }
    int indices[54], count = 0;
    for (int y0 = 0; y0 < 3; ++y0)
        for (int x0 = 0; x0 < 3; ++x0) {
            int a = y0 * 4 + x0;
            for (int n : {a, a + 1, a + 4, a + 1, a + 5, a + 4})
                indices[count++] = n;
        }
    SDL_SetTextureAlphaMod(terrain.texture, 255);
    SDL_RenderGeometry(r, terrain.texture, vertices, 16, indices, count);
}
float route(int arena, float x, float y) {
    float dx = x - 12, dy = y - 9, distance;
    switch (arena) {
    case 0:
        distance = std::max(std::abs(dx) * .65f, std::abs(dy)) - 4.3f;
        break;
    case 1:
        distance = std::abs(std::sqrt(dx * dx / 64 + dy * dy / 30) - .84f) * 5;
        break;
    case 2:
        distance = std::abs(y - (9 + 2 * std::sin(x * .28f)));
        break;
    case 3:
        distance = std::abs(std::abs(dx) + 1.3f * std::abs(dy) - 7.5f);
        break;
    case 4:
        distance = std::abs(y - (3 + x * .5f));
        break;
    default:
        distance = std::min(std::abs(y - (5.5f + std::sin(x * .4f))),
                            std::abs(y - (12.5f - std::sin(x * .4f))));
        break;
    }
    return std::clamp(1.6f - distance, 0.f, 1.f);
}
void background(const World &w) {
    const auto &style = Gardens[w.arena];
    rect(AX, AY, 24 * S, 18 * S, style.floor);
    // Seeded broad material patches, staggered rather than a repeating checkerboard.
    for (int gy = -1; gy < 6; ++gy)
        for (int gx = -1; gx < 7; ++gx) {
            uint32_t h = garden_hash(w.rng ^ uint32_t(w.arena), gx, gy);
            int x = AX + gx * 116 + int(h % 31) - 15 + (gy % 2) * 29;
            int y = AY + gy * 116 + int((h >> 8) % 25) - 12;
            soft_tile(style.base + int((h >> 12) & 1), x, y, 150, h, 140);
        }
    // Wide low-contrast paths guide composition, not navigation or collision.
    for (int y = 0; y < 18 * S; y += 8)
        for (int x = 0; x < 24 * S; x += 8) {
            float weight = route(w.arena, float(x) / S, float(y) / S);
            if (weight > 0)
                rect(AX + x, AY + y, 8, 8,
                     {uint8_t(style.floor.r + 13), uint8_t(style.floor.g + 12),
                      uint8_t(style.floor.b + 10), uint8_t(weight * 135)});
        }
    for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 6; ++x) {
            float weight = route(w.arena, x * 4.f + 2, y * 4.f + 2);
            if (weight > .3f)
                soft_tile(style.path, AX + x * 116, AY + y * 116, 116,
                          garden_hash(w.rng, x + 20, y), int(weight * 65));
        }
    // Keep decoration in the periphery and frame the fighting space with a soft value falloff.
    for (int d = 0; d < 32; d += 4) {
        int a = (32 - d) / 3;
        SDL_Color c{24, 35, 39, uint8_t(a)};
        rect(AX + d, AY + d, 24 * S - 2 * d, 4, c);
        rect(AX + d, AY + 18 * S - d - 4, 24 * S - 2 * d, 4, c);
        rect(AX + d, AY + d, 4, 18 * S - 2 * d, c);
        rect(AX + 24 * S - d - 4, AY + d, 4, 18 * S - 2 * d, c);
    }
}
void ground(const Surface &g, int tick, bool inspect) {
    static const int tile[] = {0, 16, 17, 18, 19, 20, 21, 22, 23};
    static const SDL_Color rims[] = {
        {0, 0, 0, 0},         {97, 159, 180, 190}, {156, 186, 201, 210},
        {109, 141, 107, 190}, {203, 123, 72, 210}, {147, 160, 166, 130},
        {196, 173, 99, 210},  {149, 121, 91, 190}, {147, 122, 162, 190}};
    int rad = g.radius * S / Q, cx = sx(g.pos.x), cy = sy(g.pos.y);
    auto src = terrain.cell(tile[g.kind]);
    SDL_SetTextureAlphaMod(terrain.texture, g.kind == Steam ? 140 : 230);
    // One broad material motif per patch. No repeated tiny ice cracks or water sparkles.
    for (int y = -rad; y < rad; y += 4) {
        int half = int(std::sqrt(double(std::max(0, rad * rad - y * y))));
        for (int x = -half; x < half; x += 4) {
            int u = (x + rad) * src.w / std::max(1, 2 * rad),
                v = (y + rad) * src.h / std::max(1, 2 * rad);
            SDL_Rect a{src.x + u, src.y + v, std::max(1, 4 * src.w / std::max(1, 2 * rad)),
                       std::max(1, 4 * src.h / std::max(1, 2 * rad))};
            a.w = std::min(a.w, src.x + src.w - a.x);
            a.h = std::min(a.h, src.y + src.h - a.y);
            SDL_Rect b{cx + x, cy + y, std::min(4, half - x), std::min(4, rad - y)};
            SDL_RenderCopy(r, terrain.texture, &a, &b);
        }
    }
    worldcircle(g.pos, g.radius + 35, {24, 36, 42, 120});
    worldcircle(g.pos, g.radius, rims[g.kind]);
    if ((g.kind == Water || g.kind == ChargedWater) && length(g.flow))
        for (int offset : {-1, 1})
            arrow(g.pos + Vec{offset * g.radius / 3, -g.radius / 3} + scale(g.flow, tick % 24, 1),
                  g.flow, 470, {156, 195, 209, 165});
    if (g.kind == Fire) {
        fx(1, g.pos, 25 + (tick / 4 % 3) * 2, -90, 165, false);
        fx(1, g.pos + Vec{g.radius / 3, g.radius / 3}, 18, -90, 140, false);
    }
    if (g.kind == ChargedWater && tick % 24 < 9)
        fx(2, g.pos, std::min(40, rad), 15, 155, false);
    if (inspect)
        label(cx - 12, cy + rad - 10, surface_name(g.kind), pale, 1);
    // Neutral ground lasts for the match: don't print a distracting match-length countdown.
    if ((g.effect_timer || g.owner >= 0) && inspect) {
        int life = g.effect_timer ? g.effect_timer : g.life;
        label(cx - 9, cy + rad + 3, seconds(life), pale, 1);
    } else if (g.effect_timer || g.owner >= 0) {
        int life = g.effect_timer ? g.effect_timer : g.life;
        rect(cx - 12, cy + rad - 5, 24, 3, {25, 39, 44, 175});
        rect(cx - 12, cy + rad - 5, 24 * std::min(600, life) / 600, 2, rims[g.kind]);
    }
}

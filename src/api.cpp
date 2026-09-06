#include "creature/api.h"
#include "creature/sim.hpp"
#include <algorithm>
#include <new>
using namespace creature;
static constexpr int ObsSize = ObservationSize;
extern "C" {
uint32_t cr_content_hash() {
    return ContentHash;
}
uint32_t cr_observation_version() {
    return ObservationVersion;
}
int32_t cr_arena_count() {
    return ArenaCount;
}
const char *cr_arena_name(int32_t id) {
    return arena_name(id);
}
int32_t cr_species_count() {
    return SpeciesCount;
}
const char *cr_species_name(int32_t id) {
    return id >= 0 && id < SpeciesCount ? Roster[id].name : nullptr;
}
int32_t cr_reset_match(void *ptr, uint32_t seed, int32_t weather, int32_t a, int32_t b,
                       int32_t arena) {
    if (!ptr || a < 0 || a >= 40 || b < 0 || b >= 40 || weather < 0 || weather > 2 || arena < 0 ||
        arena >= ArenaCount)
        return -1;
    reset(*static_cast<World *>(ptr), seed, weather, a, b, arena);
    return 0;
}
uint32_t cr_version() {
    return RulesVersion;
}
int32_t cr_observation_size() {
    return ObsSize;
}
void *cr_create(uint32_t seed, int32_t weather) {
    auto *w = new (std::nothrow) World;
    if (w)
        reset(*w, seed, weather);
    return w;
}
void cr_destroy(void *w) {
    delete static_cast<World *>(w);
}
int32_t cr_reset(void *w, uint32_t seed, int32_t weather) {
    if (!w)
        return -1;
    reset(*static_cast<World *>(w), seed, weather);
    return 0;
}
int32_t cr_step(void *ptr, const int32_t *a, int32_t *f, int32_t *s) {
    if (!ptr || !a || !f || !s)
        return -1;
    auto &w = *static_cast<World *>(ptr);
    std::array<Action, 2> actions;
    for (int i = 0; i < 2; i++)
        actions[i] = {a[i * 5], a[i * 5 + 1], a[i * 5 + 2], a[i * 5 + 3], a[i * 5 + 4]};
    auto r = step(w, actions);
    for (int i = 0; i < 2; i++) {
        auto &v = r.features[i];
        int32_t values[] = {v.dealt, v.taken,  v.dodged,   v.interrupts, v.ko,
                            v.death, v.healed, v.shielded, v.control,    v.spent};
        std::copy(values, values + FeatureSize, f + i * FeatureSize);
    }
    s[0] = w.terminal;
    s[1] = w.truncated;
    s[2] = w.winner;
    s[3] = r.ticks;
    return 0;
}
int32_t cr_observe(void *ptr, int32_t agent, float *out, size_t n) {
    if (!ptr || !out || n < ObsSize || agent < 0 || agent > 1)
        return -1;
    auto o = observe(*static_cast<World *>(ptr), agent);
    auto append = [&](const auto &a) { out = std::copy(a.begin(), a.end(), out); };
    append(o.self);
    append(o.entities);
    append(o.moves);
    append(o.announced);
    append(o.history);
    append(o.global);
    append(o.mask);
    return 0;
}
int32_t cr_scripted(void *w, int32_t agent, int32_t *out) {
    if (!w || !out || agent < 0 || agent > 1)
        return -1;
    auto a = scripted(*static_cast<World *>(w), agent);
    int32_t values[] = {a.mx, a.my, a.ax, a.ay, a.ability};
    std::copy(values, values + 5, out);
    return 0;
}
int32_t cr_batch_scripted(void *const *worlds, size_t count, const int32_t *styles,
                          int32_t *actions) {
    if (!worlds || !styles || !actions || count > 65536)
        return -1;
    for (size_t i = 0; i < count; ++i)
        if (!worlds[i] || styles[i * 2] < 0 || styles[i * 2] > 3 || styles[i * 2 + 1] < 0 ||
            styles[i * 2 + 1] > 3)
            return -1;
    for (size_t i = 0; i < count; ++i)
        for (int p = 0; p < 2; ++p) {
            auto a = scripted(*static_cast<World *>(worlds[i]), p, styles[i * 2 + p]);
            int32_t values[] = {a.mx, a.my, a.ax, a.ay, a.ability};
            std::copy(values, values + 5, actions + i * 10 + p * 5);
        }
    return 0;
}
int32_t cr_command(void *w, int32_t a, int32_t g) {
    if (!w || a < 0 || a > 1 || g < 0 || g > 3)
        return -1;
    command(*static_cast<World *>(w), a, g);
    return 0;
}
size_t cr_snapshot(void *w, uint8_t *out, size_t n) {
    if (!w)
        return 0;
    try {
        auto bytes = snapshot(*static_cast<World *>(w));
        if (out && n >= bytes.size())
            std::copy(bytes.begin(), bytes.end(), out);
        return bytes.size();
    } catch (...) {
        return 0;
    }
}
int32_t cr_restore(void *w, const uint8_t *p, size_t n) {
    if (!w)
        return -1;
    try {
        return restore(*static_cast<World *>(w), p, n) ? 0 : -1;
    } catch (...) {
        return -1;
    }
}
uint64_t cr_hash(void *w) {
    if (!w)
        return 0;
    try {
        return hash(*static_cast<World *>(w));
    } catch (...) {
        return 0;
    }
}
int32_t cr_batch_step(void *const *w, size_t n, const int32_t *a, float *o, int32_t *f,
                      int32_t *s) {
    if (!w || !a || !o || !f || !s || n > 65536)
        return -1;
    for (size_t i = 0; i < n; i++)
        if (!w[i])
            return -1;
    for (size_t i = 0; i < n; i++) {
        cr_step(w[i], a + i * 10, f + i * 2 * FeatureSize, s + i * 4);
        for (int j = 0; j < 2; j++)
            cr_observe(w[i], j, o + (i * 2 + j) * ObsSize, ObsSize);
    }
    return 0;
}
}

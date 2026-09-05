#include "creature/sim.hpp"
#include <algorithm>
#include <limits>
#include <type_traits>
namespace creature {
Vec operator+(Vec a, Vec b) {
    return {a.x + b.x, a.y + b.y};
}
Vec operator-(Vec a, Vec b) {
    return {a.x - b.x, a.y - b.y};
}
Vec scale(Vec v, int n, int d) {
    return {int32_t(int64_t(v.x) * n / d), int32_t(int64_t(v.y) * n / d)};
}
static int64_t dot(Vec a, Vec b) {
    return int64_t(a.x) * b.x + int64_t(a.y) * b.y;
}
static uint64_t isqrt(uint64_t n) {
    uint64_t r = 0, b = uint64_t(1) << 62;
    while (b > n)
        b >>= 2;
    while (b) {
        if (n >= r + b) {
            n -= r + b;
            r = (r >> 1) + b;
        } else
            r >>= 1;
        b >>= 2;
    }
    return r;
}
int length(Vec v) {
    return int(isqrt(uint64_t(dot(v, v))));
}
Vec unit(Vec v) {
    int n = length(v);
    return n ? scale(v, Q, n) : Vec{Q, 0};
}
static uint32_t random(World &w) {
    auto x = w.rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return w.rng = x;
}
static void event(World &w, int k, int a, int t, int m, int amount, Vec p) {
    w.events[w.event_head] = {w.tick, k, a, t, m, amount, p};
    w.event_head = (w.event_head + 1) % HistoryCount;
    w.event_count = std::min(HistoryCount, w.event_count + 1);
}
int move_id(const Body &b, int slot) {
    return slot == 4 ? 160 : slot >= 0 && slot < 4 ? Roster[b.species].moves[slot] : 160;
}
const Move &move_for(const Body &b, int slot) {
    return Moves[move_id(b, slot)];
}
Phase phase(const Body &b) {
    if (b.move < 0)
        return Idle;
    auto &m = Moves[b.move];
    return b.age < m.startup ? Startup : b.age < m.startup + m.active ? Active : Recovery;
}
void reset(World &w, uint32_t seed, int weather, int a, int b, int arena) {
    w = World{};
    w.rng = seed ? seed : 1;
    w.arena = std::clamp(arena, 0, 2);
    int jitter = int(random(w) % 2049) - 1024;
    for (int i = 0; i < 2; i++) {
        auto &body = w.bodies[i];
        body.species = std::clamp(i ? b : a, 0, 39);
        auto &s = Roster[body.species];
        body.hp = s.hp;
        body.radius = s.radius;
        body.pos = {(i ? 19 : 5) * Q, 9 * Q + (i ? -jitter : jitter)};
        body.aim = body.locked = {i ? -Q : Q, 0};
    }
    w.obstacles = {Obstacle{{12 * Q, 5 * Q}, 1200}, Obstacle{{12 * Q, 13 * Q}, 1200},
                   Obstacle{{6 * Q, 9 * Q}, 0}, Obstacle{{18 * Q, 9 * Q}, 0}};
    if (arena == 1)
        w.obstacles = {Obstacle{{8 * Q, 6 * Q}, 1000}, Obstacle{{16 * Q, 12 * Q}, 1000},
                       Obstacle{{8 * Q, 12 * Q}, 1000}, Obstacle{{16 * Q, 6 * Q}, 1000}};
    if (arena == 2)
        for (auto &o : w.obstacles)
            o.radius = 0;
    w.rain = weather == 2 ? 800 : 0;
    w.wind = weather == 1 || weather == 2 ? int(random(w) % 9) + 5 : 0;
}
void command(World &w, int a, int g) {
    if (a < 0 || a > 1 || g < 0 || g > 3 || w.terminal || w.truncated)
        return;
    auto &b = w.bodies[a];
    b.guidance = g;
    b.guidance_age =
        0; /* Guidance is private input; replay frames retain it, public history does not. */
}
std::array<int32_t, 6> action_mask(const World &w, int i) {
    std::array<int32_t, 6> r{1, 0, 0, 0, 0, 0};
    if (i < 0 || i > 1 || w.terminal || w.truncated)
        return r;
    auto &b = w.bodies[i];
    for (int j = 0; j < 5; j++) {
        auto &m = move_for(b, j);
        r[j + 1] = b.move < 0 && !b.stun && b.hp > m.health_cost && b.energy >= m.cost &&
                   !b.cooldown[j] && (!b.silence || j == 4) &&
                   (!b.root || (m.kind != Lunge && m.kind != Blink && m.kind != Evade));
    }
    return r;
}
static bool overlap(Vec a, int ar, Vec b, int br) {
    Vec d = a - b;
    return dot(d, d) <= int64_t(ar + br) * (ar + br);
}
static Vec terrain(const World &w, Vec p, int radius) {
    p.x = std::clamp(p.x, radius, 24 * Q - radius);
    p.y = std::clamp(p.y, radius, 18 * Q - radius);
    for (auto &o : w.obstacles)
        if (o.radius) {
            Vec d = p - o.pos;
            int r = radius + o.radius;
            if (length(d) < r)
                p = o.pos + scale(unit(d), r + 1);
        }
    return p;
}
static Vec travel(const World &w, Vec p, Vec delta, int radius) {
    int n = std::max(1, (length(delta) + 63) / 64);
    Vec previous{};
    for (int j = 1; j <= n; j++) {
        Vec off = scale(delta, j, n);
        p = terrain(w, p + off - previous, radius);
        previous = off;
    }
    return p;
}
static bool clear_line(const World &w, Vec a, Vec b, int radius = 0) {
    int n = std::max(1, (length(b - a) + 63) / 64);
    for (int j = 0; j <= n; j++) {
        Vec p = a + scale(b - a, j, n);
        for (auto &o : w.obstacles)
            if (o.radius && overlap(p, radius, o.pos, o.radius))
                return false;
    }
    return true;
}
Vec target_point(const World &w, int i, int move) {
    if (i < 0 || i > 1 || move < 0 || move >= MoveCount)
        return {};
    auto &b = w.bodies[i];
    auto &m = Moves[move];
    Vec p = b.pos + scale(b.locked, m.range * b.aim_scale / Q);
    return m.kind == Field || m.kind == Trap || m.kind == Turret
               ? terrain(w, p, std::max(64, m.radius))
               : p;
}
static bool in_owned_zone(const World &w, int i) {
    for (auto &z : w.zones)
        if (z.life && z.owner == i && Moves[z.move].kind == Field &&
            overlap(z.pos, Moves[z.move].radius, w.bodies[i].pos, 0))
            return true;
    return false;
}
static int projectile_radius(const Move &m) {
    return m.kind == Turret ? 160 : m.radius;
}
static void cleanse(Body &b) {
    b.burn = b.poison = b.poison_stacks = b.slow = b.root = b.silence = b.wound = 0;
}
static void heal(World &w, int i, int amount, StepResult &r) {
    auto &b = w.bodies[i];
    if (b.hp <= 0)
        return;
    if (b.wound)
        amount = (amount + 1) / 2;
    int h = std::min(Roster[b.species].hp - b.hp, amount);
    b.hp += h;
    r.features[i].healed += h;
    if (h)
        event(w, Healed, i, i, -1, h, b.pos);
}
static bool frontal(const Body &b, Vec incoming) {
    return dot(b.aim, incoming) < -int64_t(Q) * Q / 3;
}
struct Pending {
    int source, target, move, damage;
    Vec direction;
    bool direct;
};
static void add_result(StepResult &a, const StepResult &b) {
    a.ticks += b.ticks;
    for (int i = 0; i < 2; i++) {
        auto &x = a.features[i];
        auto &y = b.features[i];
        x.dealt += y.dealt;
        x.taken += y.taken;
        x.dodged += y.dodged;
        x.interrupts += y.interrupts;
        x.ko += y.ko;
        x.death += y.death;
        x.healed += y.healed;
        x.shielded += y.shielded;
        x.control += y.control;
        x.spent += y.spent;
    }
}
static StepResult tick(World &w, const std::array<Action, 2> &actions, bool trigger) {
    StepResult result;
    if (w.terminal || w.truncated)
        return result;
    result.ticks = 1;
    std::array<Pending, 192> hits{};
    int count = 0;
    auto queue = [&](int s, int t, int m, int damage, Vec d, bool direct = true) {
        if (count < int(hits.size()))
            hits[count++] = {s, t, m, damage, d, direct};
        else {
            w.overflow++;
            event(w, Overflow, s, t, m, 0, {});
        }
    };
    auto spawn = [&](int owner, int id, Vec pos, Vec aim) {
        auto &m = Moves[id];
        for (int j = 0; j < m.shots; j++) {
            Vec dir =
                unit(aim + scale(Vec{-aim.y, aim.x}, (2 * j - (m.shots - 1)) * m.spread, 2 * Q));
            bool ok = false;
            for (auto &p : w.projectiles)
                if (!p.life) {
                    int life =
                        m.lifetime ? m.lifetime : std::max(1, m.range / std::max(1, m.speed));
                    if (m.kind == Turret)
                        life = 26;
                    p = {pos, scale(dir, m.speed), pos,        life, owner, id, 0,
                         0,   m.bounces,           m.returning};
                    ok = true;
                    break;
                }
            if (!ok) {
                w.overflow++;
                event(w, Overflow, owner, owner, id, 0, pos);
            }
        }
    };
    // Accept both inputs against the SAME visible pre-tick readiness contract.
    for (int i = 0; i < 2; i++) {
        auto &b = w.bodies[i];
        auto &s = Roster[b.species];
        auto mask = action_mask(w, i);
        int slot = actions[i].ability >= 1 && actions[i].ability <= 5 ? actions[i].ability - 1 : -1;
        Vec aim{std::clamp(actions[i].ax, -Q, Q), std::clamp(actions[i].ay, -Q, Q)};
        if (b.move < 0) {
            b.aim_scale = std::min(Q, length(aim));
            if (aim.x || aim.y)
                b.aim = unit(aim);
        }
        if (trigger && slot >= 0 && mask[slot + 1]) {
            int id = move_id(b, slot);
            auto &m = Moves[id];
            b.move = id;
            b.slot = slot;
            b.age = 0;
            b.hit_mask = 0;
            b.locked = b.aim;
            b.hp -= m.health_cost;
            b.energy -= m.cost;
            result.features[i].spent += m.cost;
            b.cooldown[slot] = m.cooldown;
            if (s.passive == Cadence && b.last_slot >= 0 && slot != b.last_slot)
                b.energy = std::min(1000, b.energy + 70);
            if (s.passive == Recycle && ++b.counter % 3 == 0)
                b.energy = std::min(1000, b.energy + 180);
            if (s.passive == Rhythm) {
                b.meter = b.idle_ticks >= 21 && b.idle_ticks <= 42 ? std::min(60, b.meter + 20) : 0;
            }
            if (s.passive == Overheat)
                b.meter = std::min(100, b.meter + 20);
            if (s.passive == Purify && slot == 4)
                cleanse(b);
            b.last_slot = slot;
            b.idle_ticks = 0;
            event(w, Started, i, i, id, 0, b.pos);
        }
    }
    // Movement, terrain and body collision are resolved before any hit queries.
    for (int i = 0; i < 2; i++) {
        auto &b = w.bodies[i];
        auto &s = Roster[b.species];
        Vec input{std::clamp(actions[i].mx, -Q, Q), std::clamp(actions[i].my, -Q, Q)};
        if (length(input) > Q)
            input = unit(input);
        int speed = s.speed;
        if (b.haste)
            speed = speed * 5 / 4;
        if (b.slow)
            speed = speed * 65 / 100;
        if (s.passive == Tailwind)
            speed += std::abs(w.wind) * 2;
        if (s.passive == Rainborn && w.wetness > 300)
            speed = speed * 115 / 100;
        if (s.passive == Web && in_owned_zone(w, i))
            speed = speed * 135 / 100;
        if (s.passive == Overheat)
            speed = speed * (100 - b.meter / 3) / 100;
        if (phase(b) == Startup)
            speed = speed * 2 / 3;
        if (b.stun || b.root)
            speed = 0;
        int traction = w.wetness > 400 ? 5 : 3;
        b.vel = b.vel + scale(scale(input, speed) - b.vel, 1, traction);
        if (b.stun || b.root)
            b.vel = {};
        if (b.move >= 0 && phase(b) == Active &&
            (Moves[b.move].kind == Lunge || Moves[b.move].kind == Evade) && !b.root)
            b.vel = scale(b.locked, Moves[b.move].speed);
        Vec old = b.pos;
        b.pos = travel(w, b.pos, b.vel, b.radius);
        b.vel = b.pos - old;
        int distance = length(b.vel);
        b.stationary = distance < 20 ? std::min(600, b.stationary + 1) : 0;
        if (s.passive == Skirmish)
            b.meter = std::min(1000, b.meter + distance / 16);
    }
    for (int n = 0; n < 3; n++) {
        auto &a = w.bodies[0];
        auto &b = w.bodies[1];
        Vec d = b.pos - a.pos;
        int depth = a.radius + b.radius - length(d);
        if (depth > 0) {
            int ma = Roster[a.species].mass, mb = Roster[b.species].mass;
            Vec dir = unit(d);
            a.pos = terrain(w, a.pos - scale(dir, (depth + 2) * mb / (ma + mb)), a.radius);
            b.pos = terrain(w, b.pos + scale(dir, (depth + 2) * ma / (ma + mb)), b.radius);
        }
    }
    for (int i = 0; i < 2; i++) {
        auto &b = w.bodies[i];
        if (b.move < 0)
            continue;
        auto &m = Moves[b.move];
        auto &s = Roster[b.species];
        if (b.age == m.startup) {
            event(w, Released, i, i, b.move, 0, b.pos);
            if (m.cleanse) {
                cleanse(b);
                if (s.passive == Renewal)
                    b.haste = std::max(b.haste, 60);
                if (s.passive == Overheat)
                    b.meter = 0;
            }
            if (m.shield) {
                b.shield = std::max(b.shield, m.shield);
                b.shield_timer = 90;
                event(w, Shielded, i, i, b.move, m.shield, b.pos);
            }
            if (m.guard)
                b.guard = std::max(b.guard, m.guard);
            if (m.heal && m.kind != Field)
                heal(w, i, m.heal, result);
            if (m.haste)
                b.haste = std::max(b.haste, m.haste);
            if (m.kind == Bolt)
                spawn(i, b.move, b.pos, b.locked);
            if (m.kind == Blink) {
                b.pos = travel(w, b.pos, scale(b.locked, m.range), b.radius);
                if (s.passive == Ambush)
                    b.meter = 100;
            }
            if (m.kind == Field || m.kind == Trap || m.kind == Turret) {
                bool ok = false;
                for (auto &z : w.zones)
                    if (!z.life) {
                        z = {target_point(w, i, b.move),
                             m.lifetime,
                             i,
                             b.move,
                             0,
                             m.kind == Turret ? (s.passive == Architect ? 45 : 32)
                             : m.kind == Trap ? 12
                                              : 0};
                        ok = true;
                        break;
                    }
                if (!ok) {
                    w.overflow++;
                    event(w, Overflow, i, i, b.move, 0, b.pos);
                }
            }
            if (s.passive == Ambush && m.kind == Lunge)
                b.meter = 100;
        }
        if (phase(b) != Active)
            continue;
        auto contact = [&](Vec p, int radius) {
            int d = length(p - b.pos);
            if (d + radius < m.min_range)
                return false;
            if (m.kind == Melee || m.kind == Lunge)
                return overlap(b.pos + scale(b.locked, m.range / 2), m.radius, p, radius) &&
                       clear_line(w, b.pos, p);
            if (m.kind == Nova)
                return overlap(b.pos, m.radius, p, radius) && clear_line(w, b.pos, p);
            if (m.kind == Beam) {
                int forward = int(dot(p - b.pos, b.locked) / Q);
                Vec closest = b.pos + scale(b.locked, std::clamp(forward, 0, m.range));
                return forward >= 0 && forward <= m.range + radius &&
                       overlap(closest, m.radius, p, radius) && clear_line(w, b.pos, p);
            }
            return false;
        };
        int t = 1 - i;
        if (!(b.hit_mask & (1 << t)) && contact(w.bodies[t].pos, w.bodies[t].radius)) {
            queue(i, t, b.move, m.damage, b.locked);
            b.hit_mask |= 1 << t;
        }
        for (int j = 0; j < ZoneCount; j++) {
            auto &z = w.zones[j];
            if (z.life && z.hp > 0 && z.owner != i && !(b.hit_mask & (1 << (j + 2))) &&
                contact(z.pos, Moves[z.move].radius)) {
                queue(i, j + 2, b.move, m.damage, b.locked);
                b.hit_mask |= 1 << (j + 2);
            }
        }
    }
    for (auto &p : w.projectiles)
        if (p.life) {
            auto &m = Moves[p.move];
            int radius = projectile_radius(m);
            if (p.returning && p.age >= m.lifetime / 2) {
                if (p.returning == 1) {
                    p.hit_mask = 0;
                    p.returning = 2;
                }
                p.vel = scale(unit(w.bodies[p.owner].pos - p.pos), m.speed);
                if (overlap(p.pos, radius, w.bodies[p.owner].pos, 200)) {
                    p.life = 0;
                    continue;
                }
            }
            p.vel.x = std::clamp(p.vel.x + w.wind, -800, 800);
            Vec start = p.pos;
            int n = std::max(1, (length(p.vel) + 63) / 64);
            for (int j = 1; j <= n && p.life; j++) {
                Vec next = start + scale(p.vel, j, n);
                Vec normal{};
                bool wall = next.x < radius || next.x > 24 * Q - radius || next.y < radius ||
                            next.y > 18 * Q - radius;
                if (wall)
                    normal = next.x < radius || next.x > 24 * Q - radius ? Vec{Q, 0} : Vec{0, Q};
                for (auto &o : w.obstacles)
                    if (o.radius && overlap(next, radius, o.pos, o.radius)) {
                        wall = true;
                        normal = unit(next - o.pos);
                        break;
                    }
                if (wall) {
                    if (p.bounces > 0) {
                        p.bounces--;
                        p.vel = p.vel - scale(normal, int(2 * dot(p.vel, normal) / Q));
                        p.pos = terrain(w, p.pos, radius);
                    } else
                        p.life = 0;
                    break;
                }
                p.pos = next;
                bool covered = false;
                for (auto &z : w.zones)
                    if (z.life && z.owner != p.owner &&
                        Roster[w.bodies[z.owner].species].passive == Cover &&
                        Moves[z.move].kind == Field &&
                        overlap(next, radius, z.pos, Moves[z.move].radius)) {
                        covered = true;
                        break;
                    }
                if (covered) {
                    p.life = 0;
                    break;
                }
                for (int zindex = 0; zindex < ZoneCount; zindex++) {
                    auto &z = w.zones[zindex];
                    if (z.life && z.hp > 0 && z.owner != p.owner &&
                        !(p.hit_mask & (1 << (zindex + 2))) &&
                        overlap(next, radius, z.pos, Moves[z.move].radius)) {
                        queue(p.owner, zindex + 2, p.move, m.damage, unit(p.vel));
                        p.hit_mask |= 1 << (zindex + 2);
                        if (!m.pierce && !p.returning)
                            p.life = 0;
                        break;
                    }
                }
                if (!p.life)
                    break;
                int target = 1 - p.owner;
                auto &t = w.bodies[target];
                if (!(p.hit_mask & (1 << target)) && length(next - p.origin) >= m.min_range &&
                    overlap(next, radius, t.pos, t.radius)) {
                    if (t.guard && Roster[t.species].passive == Mirror && frontal(t, unit(p.vel))) {
                        p.owner = target;
                        p.vel = scale(p.vel, -Q);
                        p.origin = p.pos;
                        p.hit_mask = 0;
                        event(w, Parried, target, 1 - target, p.move, 0, p.pos);
                        break;
                    }
                    queue(p.owner, target, p.move, m.damage, unit(p.vel));
                    p.hit_mask |= 1 << target;
                    if (!m.pierce && !p.returning)
                        p.life = 0;
                }
            }
            if (p.life)
                p.life--;
            p.age++;
        }
    for (auto &z : w.zones)
        if (z.life) {
            auto &m = Moves[z.move];
            if (m.kind == Turret) {
                if (z.age % m.period == 0 && length(w.bodies[1 - z.owner].pos - z.pos) < 7500 &&
                    clear_line(w, z.pos, w.bodies[1 - z.owner].pos))
                    spawn(z.owner, z.move, z.pos, unit(w.bodies[1 - z.owner].pos - z.pos));
            } else if (m.kind == Trap) {
                if (z.age >= 15 && overlap(z.pos, m.radius, w.bodies[1 - z.owner].pos,
                                           w.bodies[1 - z.owner].radius)) {
                    queue(z.owner, 1 - z.owner, z.move, m.damage,
                          unit(w.bodies[1 - z.owner].pos - z.pos), false);
                    z.life = 0;
                }
            } else if (z.age % m.period == 0) {
                int t = 1 - z.owner;
                if (overlap(z.pos, m.radius, w.bodies[t].pos, w.bodies[t].radius))
                    queue(z.owner, t, z.move, m.damage, unit(w.bodies[t].pos - z.pos), false);
                if (m.heal && overlap(z.pos, m.radius, w.bodies[z.owner].pos, 0))
                    heal(w, z.owner, m.heal, result);
            }
            z.age++;
            if (z.life) {
                z.life = std::max(0, z.life - (w.rain && m.burn ? 2 : 1));
                if (!z.life && m.kind == Field &&
                    Roster[w.bodies[z.owner].species].passive == Harvest)
                    heal(w, z.owner, 9, result);
            }
        }
    std::array<bool, 2> evasive{};
    for (int i = 0; i < 2; i++)
        evasive[i] = w.bodies[i].move >= 0 && Moves[w.bodies[i].move].kind == Evade &&
                     phase(w.bodies[i]) == Active;
    // Collected contacts resolve in stable order; already-collected attacks can trade.
    for (int n = 0; n < count; n++) {
        auto &p = hits[n];
        auto &m = Moves[p.move];
        auto &a = w.bodies[p.source];
        auto &as = Roster[a.species];
        if (p.target >= 2) {
            auto &z = w.zones[p.target - 2];
            if (z.life && z.hp) {
                z.hp = std::max(0, z.hp - p.damage);
                if (!z.hp)
                    z.life = 0;
            }
            continue;
        }
        auto &t = w.bodies[p.target];
        auto &ts = Roster[t.species];
        if (evasive[p.target] && !m.pierces_evasion && m.kind != Field && m.kind != Trap) {
            result.features[p.target].dodged++;
            event(w, Dodged, p.target, p.source, p.move, 0, t.pos);
            continue;
        }
        int damage = p.damage;
        if (p.direct) {
            switch (as.passive) {
            case Cinder:
                if (t.burn)
                    damage += 6;
                break;
            case Focus:
                damage += std::min(12, a.stationary / 6);
                break;
            case Conduit:
                if (w.wetness > 300)
                    damage = damage * 115 / 100;
                break;
            case Backstab:
                if (dot(t.aim, a.pos - t.pos) < 0)
                    damage += 9;
                break;
            case Ricochet:
                if (m.kind == Bolt)
                    damage += 2;
                break;
            case Wounder:
                if (t.wound)
                    damage += 6;
                break;
            case Berserk:
                damage += ((as.hp - a.hp) * 12 / as.hp);
                break;
            case Ambush:
                if (a.meter) {
                    damage += 9;
                    a.meter = 0;
                }
                break;
            case Returner:
                if (m.returning)
                    damage += 2;
                break;
            case Resonance:
                if (++a.counter % 3 == 0)
                    damage += 12;
                break;
            case Forge:
                if (a.shield)
                    damage += 7;
                break;
            case Skirmish:
                damage += a.meter / 100;
                a.meter = 0;
                break;
            case Overheat:
                damage += a.meter / 10;
                break;
            case Retaliate:
                damage += std::min(15, a.meter);
                a.meter = 0;
                break;
            case Hunter:
                if (t.mark)
                    damage += 5;
                break;
            case Rhythm:
                damage += a.meter / 6;
                a.meter = 0;
                break;
            case Magazine:
                damage += std::min(16, a.meter / 6);
                a.meter = 0;
                break;
            case Resolve:
                if (a.meter) {
                    damage += 8;
                    a.meter = 0;
                }
                break;
            default:
                break;
            }
        }
        if (as.passive == Trapper && m.kind == Trap)
            damage += 8;
        if (m.bonus_mark && t.mark && t.mark_owner == p.source) {
            damage += m.bonus_mark;
            t.mark = 0;
        }
        if (m.execute && t.hp * 100 < ts.hp * 35)
            damage += m.execute;
        if (ts.passive == Bulwark && frontal(t, p.direction))
            damage = damage * 80 / 100;
        if (t.guard && frontal(t, p.direction)) {
            damage = damage * 60 / 100;
            if (ts.passive == Resolve)
                t.meter = 100;
        }
        int absorbed = std::min(t.shield, as.passive == Nullify ? damage * 2 : damage);
        t.shield -= absorbed;
        int blocked = as.passive == Nullify ? (absorbed + 1) / 2 : absorbed;
        damage = std::max(0, damage - blocked);
        result.features[p.target].shielded += blocked;
        if (ts.passive == Retaliate)
            t.meter = std::min(20, t.meter + blocked);
        int actual = std::min(t.hp, damage);
        t.hp -= actual;
        result.features[p.source].dealt += actual;
        result.features[p.target].taken += actual;
        event(w, Hit, p.source, p.target, p.move, actual, t.pos);
        if (as.passive == Leech && p.direct && actual)
            heal(w, p.source, std::max(1, actual / 5), result);
        if (m.burn) {
            t.burn = std::max(t.burn, m.burn);
            t.burn_owner = p.source;
        }
        if (m.poison) {
            t.poison = std::max(t.poison, m.poison);
            t.poison_owner = p.source;
            t.poison_stacks = std::min(as.passive == Venom ? 5 : 3, t.poison_stacks + 1);
        }
        if (m.wound || as.passive == Corrode)
            t.wound = std::max(t.wound, std::max(m.wound, as.passive == Corrode ? 90 : 0));
        if (m.mark) {
            t.mark = std::max(t.mark, m.mark);
            t.mark_owner = p.source;
        }
        if (m.slow)
            t.slow = std::max(t.slow, m.slow);
        if (as.passive == Frost && m.slow && ++a.counter % 3 == 0 && !t.cc_resist) {
            t.root = 15;
            t.cc_resist = 75;
        }
        if (as.passive == Tether && length(a.pos - t.pos) > 3500)
            t.slow = std::max(t.slow, 24);
        if (!t.cc_resist) {
            if (m.root) {
                t.root = std::max(t.root, m.root);
                t.cc_resist = 75;
            }
            if (m.silence) {
                t.silence = std::max(t.silence, m.silence);
                t.cc_resist = 75;
            }
        }
        if (m.drain)
            t.energy = std::max(0, t.energy - m.drain);
        if (m.impulse) {
            Vec old = t.pos;
            Vec delta = scale(p.direction, m.impulse * 100 / ts.mass);
            t.pos = travel(w, t.pos, delta, t.radius);
            bool slam = m.impulse > 0 && length(t.pos - old) + 64 < length(delta);
            if (slam) {
                int extra = std::min(t.hp, 6);
                t.hp -= extra;
                result.features[p.source].dealt += extra;
                result.features[p.target].taken += extra;
                event(w, WallSlam, p.source, p.target, p.move, extra, t.pos);
            }
        }
        bool armor = ts.passive == Resolve && t.guard;
        if (m.kind == Lunge && t.move >= 0 && phase(t) == Startup && !t.cc_resist && !armor) {
            event(w, Interrupted, p.source, p.target, t.move, 0, t.pos);
            t.move = t.slot = -1;
            t.age = 0;
            t.stun = 6;
            t.cc_resist = 75;
            result.features[p.source].interrupts++;
        }
    }
    for (int i = 0; i < 2; i++) {
        auto &b = w.bodies[i];
        auto &s = Roster[b.species];
        auto dot_damage = [&](int owner, int d) {
            int actual = std::min(b.hp, d);
            b.hp -= actual;
            result.features[i].taken += actual;
            result.features[owner].dealt += actual;
            if (actual)
                event(w, Hit, owner, i, -1, actual, b.pos);
        };
        if (w.tick % 30 == 29) {
            if (b.burn)
                dot_damage(b.burn_owner, 2);
            if (b.poison)
                dot_damage(b.poison_owner, b.poison_stacks);
            if (s.passive == Rainborn && w.wetness > 300)
                heal(w, i, 2, result);
            if (s.passive == Sanctuary && in_owned_zone(w, i))
                heal(w, i, 3, result);
        }
        if (s.passive == Anchor && b.stationary >= 30 && w.tick % 60 == 29) {
            b.shield = std::max(b.shield, 18);
            b.shield_timer = 90;
        }
        if (s.passive == Magazine && b.move < 0)
            b.meter = std::min(100, b.meter + 3);
        if (s.passive == Overheat && w.tick % 6 == 0)
            b.meter = std::max(0, b.meter - 1);
        if (b.move >= 0) {
            auto &m = Moves[b.move];
            b.age++;
            if (b.age >= m.startup + m.active + m.recovery) {
                event(w, Ended, i, i, b.move, 0, b.pos);
                b.move = b.slot = -1;
                b.age = 0;
                b.hit_mask = 0;
            }
        }
        for (auto &c : b.cooldown)
            if (c)
                c--;
        int regen = s.regen + (s.passive == Reservoir && b.guard ? 7 : 0);
        b.energy = std::min(1000, b.energy + regen);
        auto dec = [](int32_t &t) {
            if (t)
                t--;
        };
        dec(b.stun);
        dec(b.haste);
        dec(b.slow);
        dec(b.root);
        dec(b.silence);
        dec(b.wound);
        dec(b.mark);
        dec(b.guard);
        dec(b.cc_resist);
        dec(b.shield_timer);
        if (!b.shield_timer)
            b.shield = 0;
        b.burn = std::max(0, b.burn - (w.rain ? 2 : 1));
        dec(b.poison);
        if (!b.poison)
            b.poison_stacks = 0;
        b.idle_ticks = std::min(600, b.idle_ticks + 1);
        b.guidance_age = std::min(900, b.guidance_age + 1);
        if (b.guidance_age == 900)
            b.guidance = Free;
    }
    if (w.objective) {
        bool inside[2] = {length(w.bodies[0].pos - Vec{12 * Q, 9 * Q}) <= 2500,
                          length(w.bodies[1].pos - Vec{12 * Q, 9 * Q}) <= 2500};
        for (int i = 0; i < 2; i++) {
            auto &b = w.bodies[i];
            if (inside[i] && !inside[1 - i] && b.hp) {
                b.capture++;
                if (b.capture > 30) {
                    b.control++;
                    result.features[i].control++;
                    if (b.control % 30 == 0)
                        event(w, Controlled, i, i, -1, b.control, b.pos);
                }
            } else
                b.capture = 0;
            if (w.tick >= 1800 && w.tick % 15 == 0) {
                int radius = 12 * Q - (w.tick - 1800) * 7 * Q / 900;
                if (length(b.pos - Vec{12 * Q, 9 * Q}) > radius) {
                    int d = std::min(b.hp, 3);
                    b.hp -= d;
                    result.features[i].taken += d;
                }
            }
        }
    }
    for (int i = 0; i < 2; i++)
        if (w.bodies[i].hp == 0) {
            event(w, Knockout, 1 - i, i, -1, 0, w.bodies[i].pos);
            result.features[i].death++;
            result.features[1 - i].ko++;
            w.terminal = 1;
            w.end_reason = 1;
        }
    w.wetness = std::clamp(w.wetness + (w.rain ? 3 : -1), 0, 1000);
    w.tick++;
    if (w.terminal)
        w.winner = w.bodies[0].hp == w.bodies[1].hp ? -1 : (w.bodies[0].hp > 0 ? 0 : 1);
    else if (w.objective && (w.bodies[0].control >= 600 || w.bodies[1].control >= 600)) {
        w.terminal = 1;
        w.end_reason = 2;
        w.winner = w.bodies[0].control >= 600 ? 0 : 1;
    } else if (w.tick >= MaxTicks) {
        w.truncated = 1;
        w.end_reason = 3;
        int a = w.bodies[0].control, b = w.bodies[1].control;
        if (a != b)
            w.winner = a > b ? 0 : 1;
        else {
            int64_t lhs = int64_t(w.bodies[0].hp) * Roster[w.bodies[1].species].hp,
                    rhs = int64_t(w.bodies[1].hp) * Roster[w.bodies[0].species].hp;
            w.winner = lhs == rhs ? -1 : lhs > rhs ? 0 : 1;
        }
    }
    return result;
}
StepResult step(World &w, const std::array<Action, 2> &a, int ticks) {
    StepResult out;
    for (int t = 0; t < std::clamp(ticks, 0, DecisionTicks); t++)
        add_result(out, tick(w, a, t == 0));
    return out;
}
Action scripted(const World &w, int i, int style) {
    if (i < 0 || i > 1)
        return {};
    auto &b = w.bodies[i];
    auto &e = w.bodies[1 - i];
    auto &s = Roster[b.species];
    Vec target = e.pos;
    Vec enemy_velocity = e.vel;
    // Targetable enemy setup is public state, never an opaque species lookup advantage.
    for (auto &z : w.zones)
        if (z.life && z.hp > 0 && z.owner != i && length(z.pos - b.pos) < 6000 &&
            clear_line(w, b.pos, z.pos)) {
            target = z.pos;
            enemy_velocity = {};
            break;
        }
    Vec d = target - b.pos;
    int distance = length(d), desired = s.preferred_range;
    if (style == 1)
        desired = desired * 75 / 100;
    if (style == 2)
        desired = desired * 115 / 100;
    if (b.guidance == Retreat)
        desired = 8500;
    if (b.guidance == Attack)
        desired = 2000;
    Vec dir = unit(d), movement = dir;
    bool good = distance > desired - 600 && distance < desired + 800;
    if (distance < desired)
        movement = scale(dir, -Q);
    else if (good)
        movement = {-dir.y, dir.x};
    if (good && (s.passive == Anchor || s.passive == Focus))
        movement = {};
    if (w.objective &&
        (distance > 7000 || e.control > b.control + 60 || style == 3 || w.tick > 1500)) {
        Vec center = Vec{12 * Q, 9 * Q} - b.pos;
        if (length(center) > 1900)
            movement = unit(center);
        else if (distance > desired + 800)
            movement = unit(d);
        else if (distance > 2500)
            movement = {};
    }
    for (auto &o : w.obstacles)
        if (o.radius && length(o.pos - b.pos) < o.radius + b.radius + 1000) {
            Vec away = unit(b.pos - o.pos);
            movement = unit(movement + scale(away, 2, 1));
        }
    for (auto &z : w.zones)
        if (z.life && z.owner != i && Moves[z.move].kind != Turret &&
            length(z.pos - b.pos) < Moves[z.move].radius + b.radius + 200)
            movement = unit(b.pos - z.pos);
    bool danger = e.move >= 0 && phase(e) == Startup && distance < 4500;
    for (auto &p : w.projectiles)
        if (p.life && p.owner != i && length(p.pos - b.pos) < 2300 && dot(p.vel, b.pos - p.pos) > 0)
            danger = true;
    auto mask = action_mask(w, i);
    int ability = 0, best = -999;
    Vec chosen = dir;
    for (int slot = 0; slot < 4; slot++)
        if (mask[slot + 1]) {
            auto &m = move_for(b, slot);
            int score = m.damage * 3 - m.cost / 20;
            bool viable = false;
            Vec aim =
                unit(d + scale(enemy_velocity, m.startup + (m.speed ? distance / m.speed : 0), 1));
            if (m.kind == Melee)
                viable = distance < m.range / 2 + m.radius + e.radius - 80;
            if (m.kind == Lunge) {
                viable = distance < m.speed * m.active + m.range / 2 + m.radius + e.radius - 100;
                score -= 10;
            }
            if (m.kind == Bolt || m.kind == Beam)
                viable = distance >= m.min_range + e.radius && distance < m.range &&
                         clear_line(w, b.pos, target);
            if (m.shots > 1) {
                int pellets = 0;
                for (int shot = 0; shot < m.shots; shot++)
                    if (std::abs((2 * shot - (m.shots - 1)) * m.spread * distance / (2 * Q)) <
                        m.radius + e.radius)
                        pellets++;
                score += m.damage * (pellets - 1);
                viable = viable && pellets > 0;
            }
            if (m.kind == Nova)
                viable = distance < m.radius + e.radius - 100 && distance > m.min_range;
            if (m.kind == Field || m.kind == Trap) {
                viable = distance < m.range + m.radius - 100;
                aim = scale(unit(d + scale(enemy_velocity, 8, 1)),
                            std::min(Q, distance * Q / std::max(1, m.range)));
                if (m.heal)
                    aim = scale(unit(d), std::min(Q, (std::min(distance / 2, m.radius / 2)) * Q /
                                                         std::max(1, m.range)));
                score += m.kind == Field ? m.damage * 7 : 15;
                int owned = 0;
                for (auto &z : w.zones)
                    if (z.life && z.owner == i && z.move == move_id(b, slot))
                        owned++;
                score -= owned * 45;
            }
            if (m.kind == Turret) {
                viable = distance < 10000;
                aim = scale(dir, 600);
                score = 40;
                int owned = 0;
                for (auto &z : w.zones)
                    if (z.life && z.owner == i && Moves[z.move].kind == Turret)
                        owned++;
                score -= owned * 70;
            }
            if (m.kind == Ward) {
                viable =
                    (danger || distance < 6500 || b.hp < s.hp - m.heal) && b.shield < m.shield + 1;
                score = (danger ? 42 : 0) + (s.hp - b.hp >= m.heal ? m.heal * 2 : 0) +
                        (b.shield == 0 ? m.shield : 0) - 10;
                if (m.cleanse && (b.burn || b.poison || b.slow))
                    score += 35;
                if (s.passive == Forge || s.passive == Reservoir || s.passive == Retaliate)
                    score += 18;
                if (!m.shield && !m.heal && !m.cleanse)
                    viable = distance < 4000;
            }
            if (m.kind == Blink) {
                viable = distance > desired + 2500 || (danger && distance < 3000);
                aim = distance > desired + 2500 ? dir : scale(dir, -Q);
                score = 20;
            }
            if (m.mark && !e.mark)
                score += 28;
            if (m.bonus_mark)
                score += e.mark ? 25 : -8;
            if (m.burn && !e.burn)
                score += 8;
            if (m.poison && e.poison < 45)
                score += 8;
            if (s.passive == Magazine && m.damage && b.meter < 60 && distance > 3500)
                viable = false;
            if (b.guidance == Conserve && m.cost > 150)
                viable = false;
            if (viable && score > best) {
                best = score;
                ability = slot + 1;
                chosen = aim;
            }
        }
    if (danger && mask[5] && (best < 65 || b.hp * 2 < s.hp)) {
        ability = 5;
        chosen = unit(Vec{-d.y, d.x});
    }
    return {movement.x, movement.y, chosen.x, chosen.y, ability};
}
Observation observe(const World &w, int i) {
    Observation o;
    if (i < 0 || i > 1)
        return o;
    auto &b = w.bodies[i];
    auto &s = Roster[b.species];
    auto &e = w.bodies[1 - i];
    auto relative = [&](Vec v) {
        return std::array<float, 2>{float(dot(v, b.aim)) / (Q * 24 * Q),
                                    float(int64_t(v.y) * b.aim.x - int64_t(v.x) * b.aim.y) /
                                        (Q * 24 * Q)};
    };
    o.self = {float(b.hp) / s.hp,
              float(b.energy) / 1000,
              float(b.vel.x) / 512,
              float(b.vel.y) / 512,
              float(b.aim.x) / Q,
              float(b.aim.y) / Q,
              float(b.move + 1) / MoveCount,
              float(phase(b)) / 3,
              float(b.age) / 90,
              float(b.stun) / 30,
              float(b.burn) / 150,
              float(b.haste) / 150,
              float(b.radius) / Q,
              float(b.pos.x) / (24 * Q),
              float(b.pos.y) / (18 * Q),
              float(b.guidance) / 3,
              float(b.guidance_age) / 900};
    for (int k = 0; k < 5; k++)
        o.self[17 + k] = float(b.cooldown[k]) / move_for(b, k).cooldown;
    o.self[22] = float(b.species) / 39;
    o.self[23] = float(s.passive) / 39;
    o.self[24] = float(b.shield) / 60;
    o.self[25] = float(b.guard) / 60;
    o.self[26] = float(b.poison) / 150;
    o.self[27] = float(b.poison_stacks) / 5;
    o.self[28] = float(b.slow) / 90;
    o.self[29] = float(b.root) / 30;
    o.self[30] = float(b.silence) / 30;
    o.self[31] = float(b.wound) / 150;
    o.self[32] = float(b.mark) / 150;
    o.self[33] = float(b.cc_resist) / 75;
    o.self[34] = float(b.meter) / 1000;
    o.self[35] = float(b.counter % 3) / 2;
    o.self[36] = float(b.idle_ticks) / 90;
    o.self[37] = float(b.stationary) / 90;
    o.self[38] = float(b.control) / 600;
    o.self[39] = float(s.speed) / 210;
    o.self[40] = float(s.hp) / 180;
    o.self[41] = float(s.regen) / 12;
    o.self[42] = float(s.mass) / 200;
    for (int k = 0; k < 8; k++)
        o.self[43 + k] = float(s.axes[k]) / 5;
    o.self[51] = float(b.last_slot + 1) / 5;
    o.self[52] = float(b.aim_scale) / Q;
    o.self[53] = float(b.shield_timer) / 90;
    o.self[54] = float(b.slot + 1) / 5;
    o.self[55] = float(b.passive_timer) / 90;
    auto entity = [&](int row, int kind, Vec pos, Vec vel, int radius, int team, int life, int move,
                      int ph, int age, float hp) {
        float *p = o.entities.data() + row * EntitySize;
        auto rp = relative(pos - b.pos), rv = relative(vel);
        p[0] = float(kind) / 4;
        p[1] = rp[0];
        p[2] = rp[1];
        p[3] = rv[0] * 30;
        p[4] = rv[1] * 30;
        p[5] = float(radius) / (24 * Q);
        p[6] = float(team);
        p[7] = float(life) / 300;
        p[8] = float(move + 1) / MoveCount;
        p[9] = float(ph) / 3;
        p[10] = float(age) / 90;
        p[11] = hp;
        Vec facing = kind == 1 ? (e.move >= 0 ? e.locked : e.aim) : kind == 3 ? unit(vel) : Vec{};
        p[12] = float(dot(facing, b.aim)) / (Q * Q);
        p[13] = float(int64_t(facing.y) * b.aim.x - int64_t(facing.x) * b.aim.y) / (Q * Q);
        p[31] = 1;
    };
    entity(0, 1, e.pos, e.vel, e.radius, -1, 0, e.move, phase(e), e.age,
           float(e.hp) / Roster[e.species].hp);
    auto *p = o.entities.data();
    p[14] = float(e.shield) / 60;
    p[15] = float(e.guard) / 60;
    p[16] = float(e.burn) / 150;
    p[17] = float(e.poison_stacks) / 5;
    p[18] = float(e.slow) / 90;
    p[19] = float(e.mark) / 150;
    p[20] = float(e.species) / 39;
    p[21] = float(e.meter) / 1000;
    p[22] = float(e.cc_resist) / 75;
    p[23] = float(e.counter % 3) / 2;
    p[24] = float(e.root) / 30;
    p[25] = float(e.silence) / 30;
    p[26] = float(e.wound) / 150;
    p[27] = float(e.haste) / 150;
    p[28] = float(e.poison) / 150;
    p[29] = float(e.stun) / 30;
    p[30] = float(e.aim_scale) / Q;
    for (int j = 0; j < 4; j++)
        if (w.obstacles[j].radius)
            entity(1 + j, 2, w.obstacles[j].pos, {}, w.obstacles[j].radius, 0, 0, -1, 0, 0, 0);
    for (int j = 0; j < ProjectileCount; j++) {
        auto &v = w.projectiles[j];
        if (v.life) {
            entity(5 + j, 3, v.pos, v.vel, projectile_radius(Moves[v.move]), v.owner == i ? 1 : -1,
                   v.life, v.move, 0, v.age, 0);
            o.entities[(5 + j) * EntitySize + 14] = float(v.returning);
            o.entities[(5 + j) * EntitySize + 15] = float(v.bounces) / 3;
        }
    }
    for (int j = 0; j < ZoneCount; j++) {
        auto &z = w.zones[j];
        if (z.life) {
            entity(37 + j, 4, z.pos, {}, Moves[z.move].radius, z.owner == i ? 1 : -1, z.life,
                   z.move, 0, z.age, float(z.hp) / 45);
            o.entities[(37 + j) * EntitySize + 14] = float(Moves[z.move].kind) / 10;
        }
    }
    for (int j = 0; j < 6; j++) {
        if (j == 5 && e.move < 0)
            continue;
        auto &m = j == 5 ? Moves[e.move] : move_for(b, j);
        float *mp = j == 5 ? o.announced.data() : o.moves.data() + j * MoveSize;
        float values[] = {float(m.kind) / 10,
                          float(m.startup) / 30,
                          float(m.active) / 30,
                          float(m.recovery) / 30,
                          float(m.cooldown) / 180,
                          float(m.cost) / 1000,
                          float(m.damage) / 60,
                          float(m.range) / (24 * Q),
                          float(m.radius) / (4 * Q),
                          float(m.speed) / 800,
                          float(m.impulse) / 1600,
                          float(m.burn) / 150,
                          float(m.haste) / 150,
                          float(m.pierces_evasion),
                          float(m.min_range) / (24 * Q),
                          float(m.lifetime) / 300,
                          float(m.period) / 90,
                          float(m.shots) / 4,
                          float(m.spread) / 400,
                          float(m.slow) / 90,
                          float(m.root) / 30,
                          float(m.silence) / 30,
                          float(m.poison) / 150,
                          float(m.wound) / 150,
                          float(m.mark) / 150,
                          float(m.shield) / 60,
                          float(m.heal) / 30,
                          float(m.guard) / 60,
                          float(m.cleanse),
                          float(m.drain) / 300,
                          float(m.bonus_mark) / 30,
                          float(m.execute) / 30,
                          float(m.health_cost) / 20,
                          float(m.bounces) / 3,
                          float(m.returning),
                          float(m.pierce),
                          1,
                          0,
                          0,
                          0};
        std::copy(std::begin(values), std::end(values), mp);
    }
    int visible = 0;
    for (int j = 0; j < w.event_count && visible < EventCount; j++) {
        auto &ev = w.events[(w.event_head - 1 - j + HistoryCount) % HistoryCount];
        if (ev.kind == Command && ev.actor != i)
            continue;
        auto *ep = o.history.data() + visible++ * EventSize;
        ep[0] = float(w.tick - ev.tick) / 150;
        ep[1] = float(ev.kind) / 15;
        ep[2] = ev.actor == i ? 1 : -1;
        ep[3] = ev.target == i ? 1 : -1;
        ep[4] = float(ev.move + 1) / MoveCount;
        ep[5] = float(ev.amount) / 180;
        ep[6] = 1;
        ep[7] = 1;
    }
    o.global = {float(w.tick) / MaxTicks,
                float(w.rain) / 1000,
                float(w.wind) / 16,
                float(w.wetness) / 1000,
                float(w.terminal),
                float(w.truncated),
                float(w.arena) / 2,
                float(w.objective),
                float(b.control) / 600,
                float(e.control) / 600,
                float(b.capture) / 30,
                float(e.capture) / 30,
                float(w.end_reason) / 3,
                0,
                0,
                0};
    auto mask = action_mask(w, i);
    for (int j = 0; j < 6; j++)
        o.mask[j] = float(mask[j]);
    return o;
}
// Every persisted field is encoded explicitly; no padding/pointers/host endianness.
template <class F> static void fields(World &w, F f) {
    f(w.rng);
    f(w.tick);
    f(w.terminal);
    f(w.truncated);
    f(w.winner);
    f(w.end_reason);
    f(w.rain);
    f(w.wind);
    f(w.wetness);
    f(w.event_head);
    f(w.event_count);
    f(w.overflow);
    f(w.arena);
    f(w.objective);
    auto vec = [&](Vec &v) {
        f(v.x);
        f(v.y);
    };
    for (auto &b : w.bodies) {
        vec(b.pos);
        vec(b.vel);
        vec(b.aim);
        vec(b.locked);
        f(b.species);
        f(b.aim_scale);
        f(b.hp);
        f(b.energy);
        f(b.radius);
        f(b.move);
        f(b.slot);
        f(b.age);
        f(b.hit_mask);
        f(b.stun);
        f(b.burn);
        f(b.burn_owner);
        f(b.haste);
        f(b.poison);
        f(b.poison_owner);
        f(b.poison_stacks);
        f(b.slow);
        f(b.root);
        f(b.silence);
        f(b.wound);
        f(b.mark);
        f(b.mark_owner);
        f(b.guard);
        f(b.shield);
        f(b.shield_timer);
        f(b.cc_resist);
        f(b.meter);
        f(b.counter);
        f(b.passive_timer);
        f(b.last_slot);
        f(b.idle_ticks);
        f(b.stationary);
        f(b.control);
        f(b.capture);
        for (auto &c : b.cooldown)
            f(c);
        f(b.guidance);
        f(b.guidance_age);
    }
    for (auto &p : w.projectiles) {
        vec(p.pos);
        vec(p.vel);
        vec(p.origin);
        f(p.life);
        f(p.owner);
        f(p.move);
        f(p.age);
        f(p.hit_mask);
        f(p.bounces);
        f(p.returning);
    }
    for (auto &z : w.zones) {
        vec(z.pos);
        f(z.life);
        f(z.owner);
        f(z.move);
        f(z.age);
        f(z.hp);
    }
    for (auto &o : w.obstacles) {
        vec(o.pos);
        f(o.radius);
    }
    for (auto &e : w.events) {
        f(e.tick);
        f(e.kind);
        f(e.actor);
        f(e.target);
        f(e.move);
        f(e.amount);
        vec(e.pos);
    }
}
static uint64_t checksum(const uint8_t *p, size_t n) {
    uint64_t h = 14695981039346656037ull;
    for (size_t i = 0; i < n; i++) {
        h ^= p[i];
        h *= 1099511628211ull;
    }
    return h;
}
std::vector<uint8_t> snapshot(const World &world) {
    World w = world;
    std::vector<uint8_t> out{'C', 'R', 'L', 'B'};
    auto emit = [&](auto v) {
        uint32_t n = uint32_t(v);
        for (int j = 0; j < 4; j++)
            out.push_back(uint8_t(n >> (8 * j)));
    };
    emit(RulesVersion);
    emit(ContentHash);
    fields(w, emit);
    auto h = checksum(out.data(), out.size());
    for (int j = 0; j < 8; j++)
        out.push_back(uint8_t(h >> (8 * j)));
    return out;
}
static bool valid(const World &w) {
    if (!w.rng || w.tick < 0 || w.tick > MaxTicks || w.terminal < 0 || w.terminal > 1 ||
        w.truncated < 0 || w.truncated > 1 || w.winner < -1 || w.winner > 1 || w.end_reason < 0 ||
        w.end_reason > 3 || w.event_head < 0 || w.event_head >= HistoryCount || w.event_count < 0 ||
        w.event_count > HistoryCount || w.wind < -16 || w.wind > 16 || w.rain < 0 ||
        w.rain > 1000 || w.wetness < 0 || w.wetness > 1000 || w.overflow < 0 ||
        w.overflow > 100000 || w.arena < 0 || w.arena > 2 || w.objective < 0 || w.objective > 1)
        return false;
    auto pos = [](Vec v) { return v.x >= -Q && v.x <= 25 * Q && v.y >= -Q && v.y <= 19 * Q; };
    auto vel = [](Vec v) { return v.x >= -2 * Q && v.x <= 2 * Q && v.y >= -2 * Q && v.y <= 2 * Q; };
    for (auto &b : w.bodies) {
        if (b.species < 0 || b.species >= 40 || b.move < -1 || b.move >= MoveCount || b.slot < -1 ||
            b.slot > 4 || b.last_slot < -1 || b.last_slot > 4 || b.hp < 0 ||
            b.hp > Roster[b.species].hp || b.radius != Roster[b.species].radius || b.energy < 0 ||
            b.energy > 1000 || !pos(b.pos) || !vel(b.vel) || !vel(b.aim) || !vel(b.locked) ||
            b.age < 0 || b.age > 120 || b.aim_scale < 0 || b.aim_scale > Q || b.meter < 0 ||
            b.meter > 1000 || b.counter < 0 || b.counter > 10000 || b.poison_stacks < 0 ||
            b.poison_stacks > 5 || b.control < 0 || b.control > 600 || b.capture < 0 ||
            b.capture > MaxTicks || b.hit_mask < 0 || b.hit_mask >= (1 << (ZoneCount + 2)) ||
            b.burn_owner < 0 || b.burn_owner > 1 || b.poison_owner < 0 || b.poison_owner > 1 ||
            b.mark_owner < 0 || b.mark_owner > 1 || b.guidance < 0 || b.guidance > 3 ||
            b.guidance_age < 0 || b.guidance_age > 900)
            return false;
        const int timers[] = {b.stun,      b.burn,          b.haste,      b.poison,
                              b.slow,      b.root,          b.silence,    b.wound,
                              b.mark,      b.guard,         b.shield,     b.shield_timer,
                              b.cc_resist, b.passive_timer, b.idle_ticks, b.stationary};
        for (int t : timers)
            if (t < 0 || t > 600)
                return false;
        for (int c : b.cooldown)
            if (c < 0 || c > 600)
                return false;
        if (b.move < 0) {
            if (b.slot != -1 || b.age != 0)
                return false;
        } else if (b.slot < 0 || move_id(b, b.slot) != b.move ||
                   b.age >= Moves[b.move].startup + Moves[b.move].active + Moves[b.move].recovery)
            return false;
    }
    for (auto &p : w.projectiles)
        if (!pos(p.pos) || !pos(p.origin) || !vel(p.vel) || p.life < 0 || p.life > 600 ||
            p.age < 0 || p.age > 600 || p.owner < 0 || p.owner > 1 || p.move < 0 ||
            p.move >= MoveCount || p.bounces < 0 || p.bounces > 3 || p.returning < 0 ||
            p.returning > 2 || p.hit_mask < 0 || p.hit_mask >= (1 << (ZoneCount + 2)))
            return false;
    for (auto &z : w.zones)
        if (!pos(z.pos) || z.life < 0 || z.life > 600 || z.owner < 0 || z.owner > 1 || z.move < 0 ||
            z.move >= MoveCount || z.age < 0 || z.age > 600 || z.hp < 0 || z.hp > 60)
            return false;
    for (auto &o : w.obstacles)
        if (!pos(o.pos) || o.radius < 0 || o.radius > 2 * Q)
            return false;
    for (auto &e : w.events)
        if (e.tick < 0 || e.tick > MaxTicks || e.kind < 0 || e.kind > 15 || e.actor < 0 ||
            e.actor > 1 || e.target < 0 || e.target >= ZoneCount + 2 || e.move < -1 ||
            e.move >= MoveCount || e.amount < 0 || e.amount > 10000 || !pos(e.pos))
            return false;
    return true;
}
bool restore(World &w, const uint8_t *p, size_t n) {
    if (!p || n != snapshot(World{}).size() || p[0] != 'C' || p[1] != 'R' || p[2] != 'L' ||
        p[3] != 'B')
        return false;
    size_t off = 4;
    auto read = [&]() {
        uint32_t v = 0;
        for (int j = 0; j < 4; j++)
            v |= uint32_t(p[off++]) << (8 * j);
        return v;
    };
    if (read() != RulesVersion || read() != ContentHash)
        return false;
    uint64_t h = 0;
    for (int j = 0; j < 8; j++)
        h |= uint64_t(p[n - 8 + j]) << (8 * j);
    if (h != checksum(p, n - 8))
        return false;
    World candidate;
    fields(candidate, [&](auto &v) {
        uint32_t raw = read();
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_signed_v<T>)
            v = raw <= uint32_t(INT32_MAX) ? int32_t(raw) : int32_t(int64_t(raw) - 4294967296ll);
        else
            v = raw;
    });
    if (!valid(candidate))
        return false;
    w = candidate;
    return true;
}
uint64_t hash(const World &w) {
    auto bytes = snapshot(w);
    return checksum(bytes.data(), bytes.size() - 8);
}
} // namespace creature

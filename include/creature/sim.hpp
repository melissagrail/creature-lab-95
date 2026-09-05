#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace creature {
constexpr int Q = 1024, Hz = 30, DecisionTicks = 3, MaxTicks = 2700;
constexpr int SpeciesCount = 40, MoveCount = 161, ProjectileCount = 32, ZoneCount = 16,
              HistoryCount = 64;
constexpr uint32_t RulesVersion = 2, ObservationVersion = 2;
struct Vec {
    int32_t x = 0, y = 0;
};
Vec operator+(Vec, Vec);
Vec operator-(Vec, Vec);
Vec scale(Vec, int, int = Q);
int length(Vec);
Vec unit(Vec);
enum MoveKind { Melee, Bolt, Lunge, Field, Evade, Beam, Nova, Trap, Ward, Blink, Turret };
enum Phase { Idle, Startup, Active, Recovery };
enum Guidance { Free, Attack, Retreat, Conserve };
enum EventKind {
    Started = 1,
    Released,
    Hit,
    Dodged,
    Interrupted,
    Knockout,
    Command,
    Overflow,
    Ended,
    Healed,
    Shielded,
    Controlled,
    Parried,
    Status,
    WallSlam
};
enum Passive {
    Cinder,
    Anchor,
    Focus,
    Conduit,
    Venom,
    Bulwark,
    Frost,
    Backstab,
    Mirror,
    Rainborn,
    Trapper,
    Renewal,
    Ambush,
    Ricochet,
    Wounder,
    Sanctuary,
    Tailwind,
    Berserk,
    Cadence,
    Nullify,
    Architect,
    Returner,
    Reservoir,
    Leech,
    Resonance,
    Forge,
    Skirmish,
    Tether,
    Overheat,
    Retaliate,
    Recycle,
    Harvest,
    Cover,
    Hunter,
    Corrode,
    Purify,
    Rhythm,
    Web,
    Magazine,
    Resolve
};
struct Move {
    const char *name;
    int32_t kind, startup, active, recovery, cooldown, cost, damage, range, radius, speed, impulse,
        burn, haste, pierces_evasion;
    int32_t min_range = 0, lifetime = 0, period = 15, shots = 1, spread = 0, slow = 0, root = 0,
            silence = 0, poison = 0, wound = 0, mark = 0, shield = 0, heal = 0, guard = 0,
            cleanse = 0, drain = 0, bonus_mark = 0, execute = 0, health_cost = 0, bounces = 0,
            returning = 0, pierce = 0;
};
struct Species {
    const char *name;
    const char *role;
    const char *identity;
    int32_t hp, speed, radius, regen, mass, passive, preferred_range;
    std::array<int32_t, 4> moves;
    std::array<int32_t, 8> axes;
};
extern const std::array<Move, MoveCount> Moves;
extern const std::array<Species, SpeciesCount> Roster;
extern const uint32_t ContentHash;
struct Action {
    int32_t mx = 0, my = 0, ax = Q, ay = 0, ability = 0;
};
struct Body {
    Vec pos{}, vel{}, aim{Q, 0}, locked{Q, 0};
    int32_t species = 0, aim_scale = Q, hp = 120, energy = 1000, radius = 460, move = -1, slot = -1,
            age = 0, hit_mask = 0;
    int32_t stun = 0, burn = 0, burn_owner = 0, haste = 0, poison = 0, poison_owner = 0,
            poison_stacks = 0;
    int32_t slow = 0, root = 0, silence = 0, wound = 0, mark = 0, mark_owner = 0, guard = 0,
            shield = 0, shield_timer = 0, cc_resist = 0;
    int32_t meter = 0, counter = 0, passive_timer = 0, last_slot = -1, idle_ticks = 0,
            stationary = 0, control = 0, capture = 0;
    std::array<int32_t, 5> cooldown{};
    int32_t guidance = Free, guidance_age = 0;
};
struct Projectile {
    Vec pos{}, vel{}, origin{};
    int32_t life = 0, owner = 0, move = 0, age = 0, hit_mask = 0, bounces = 0, returning = 0;
};
struct Zone {
    Vec pos{};
    int32_t life = 0, owner = 0, move = 0, age = 0, hp = 0;
};
struct Obstacle {
    Vec pos{};
    int32_t radius = Q;
};
struct Event {
    int32_t tick = 0, kind = 0, actor = 0, target = 0, move = 0, amount = 0;
    Vec pos{};
};
struct World {
    uint32_t rng = 1;
    int32_t tick = 0, terminal = 0, truncated = 0, winner = -1, end_reason = 0;
    int32_t rain = 0, wind = 0, wetness = 0, event_head = 0, event_count = 0, overflow = 0,
            arena = 0, objective = 1;
    std::array<Body, 2> bodies{};
    std::array<Projectile, ProjectileCount> projectiles{};
    std::array<Zone, ZoneCount> zones{};
    std::array<Obstacle, 4> obstacles{};
    std::array<Event, HistoryCount> events{};
};
struct Features {
    int32_t dealt = 0, taken = 0, dodged = 0, interrupts = 0, ko = 0, death = 0, healed = 0,
            shielded = 0, control = 0, spent = 0;
};
constexpr int FeatureSize = 10;
struct StepResult {
    std::array<Features, 2> features{};
    int32_t ticks = 0;
};
void reset(World &, uint32_t, int weather = 0, int species_a = 0, int species_b = 1, int arena = 0);
const Move &move_for(const Body &, int slot);
int move_id(const Body &, int slot);
Phase phase(const Body &);
Vec target_point(const World &, int, int);
void command(World &, int, int);
std::array<int32_t, 6> action_mask(const World &, int);
StepResult step(World &, const std::array<Action, 2> &, int ticks = DecisionTicks);
Action scripted(const World &, int, int style = 0);
constexpr int SelfSize = 56, EntityCount = 53, EntitySize = 32, MoveSize = 40, EventSize = 8,
              EventCount = 24, GlobalSize = 16;
constexpr int ObservationSize =
    SelfSize + EntityCount * EntitySize + 6 * MoveSize + EventCount * EventSize + GlobalSize + 6;
struct Observation {
    std::array<float, SelfSize> self{};
    std::array<float, EntityCount * EntitySize> entities{};
    std::array<float, 5 * MoveSize> moves{};
    std::array<float, MoveSize> announced{};
    std::array<float, EventCount * EventSize> history{};
    std::array<float, GlobalSize> global{};
    std::array<float, 6> mask{};
};
Observation observe(const World &, int);
std::vector<uint8_t> snapshot(const World &);
bool restore(World &, const uint8_t *, size_t);
uint64_t hash(const World &);
} // namespace creature

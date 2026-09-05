#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace creature {
constexpr int Q=1024, Hz=30, DecisionTicks=3, MaxTicks=2700;
constexpr int ProjectileCount=16, ZoneCount=8, HistoryCount=32;
constexpr uint32_t RulesVersion=1, ObservationVersion=1;
struct Vec { int32_t x=0,y=0; };
Vec operator+(Vec a,Vec b); Vec operator-(Vec a,Vec b);
Vec scale(Vec v,int n,int d=Q); int length(Vec v); Vec unit(Vec v);
enum MoveKind { Melee, Bolt, Lunge, Field, Evade };
enum Phase { Idle, Startup, Active, Recovery };
enum Guidance { Free, Attack, Retreat, Conserve };
enum EventKind { Started=1, Released, Hit, Dodged, Interrupted, Knockout, Command, Overflow, Ended };
struct Move {
    const char* name; int32_t kind,startup,active,recovery,cooldown,cost;
    int32_t damage,range,radius,speed,impulse,burn,haste,pierces_evasion;
};
extern const std::array<Move,5> Moves;
struct Action { int32_t mx=0,my=0,ax=Q,ay=0,ability=0; }; // ability 0=no-op, 1..5
struct Body {
    Vec pos{},vel{},aim{Q,0},locked{Q,0};
    int32_t hp=100,energy=1000,radius=460;
    int32_t move=-1,age=0,hit_mask=0,stun=0,burn=0,burn_owner=0,haste=0;
    std::array<int32_t,5> cooldown{};
    int32_t guidance=Free,guidance_age=0;
};
struct Projectile { Vec pos{},vel{}; int32_t life=0,owner=0,move=1; };
struct Zone { Vec pos{}; int32_t life=0,owner=0,move=3,age=0; };
struct Obstacle { Vec pos{}; int32_t radius=Q; };
struct Event { int32_t tick=0,kind=0,actor=0,target=0,move=0,amount=0; Vec pos{}; };
struct World {
    uint32_t rng=1; int32_t tick=0,terminal=0,truncated=0,winner=-1;
    int32_t rain=0,wind=0,wetness=0,event_head=0,event_count=0,overflow=0;
    std::array<Body,2> bodies{};
    std::array<Projectile,ProjectileCount> projectiles{};
    std::array<Zone,ZoneCount> zones{};
    std::array<Obstacle,2> obstacles{};
    std::array<Event,HistoryCount> events{};
};
struct Features { int32_t dealt=0,taken=0,dodged=0,interrupts=0,ko=0,death=0; };
struct StepResult { std::array<Features,2> features{}; int32_t ticks=0; };
// Public arena: x=0..24, y=0..18. All spatial state is integer Q units.
void reset(World&,uint32_t seed,int weather=0);
Phase phase(const Body&);
Vec target_point(const World&,int agent,int move);
void command(World&,int agent,int guidance);
std::array<int32_t,6> action_mask(const World&,int agent);
StepResult step(World&,const std::array<Action,2>&,int ticks=DecisionTicks);
// Baseline receives the public observation contract: no opponent cooldown/energy access.
Action scripted(const World&,int agent);
// Fixed-shape actor tensors. Row 0 of entities is the opponent, then obstacles,
// projectile slots and zone slots. Final column is presence; zeros are padding.
constexpr int SelfSize=24, EntityCount=27, EntitySize=16, MoveSize=16, EventSize=8, EventCount=16, GlobalSize=8;
struct Observation {
    std::array<float,SelfSize> self{};
    std::array<float,EntityCount*EntitySize> entities{};
    std::array<float,5*MoveSize> moves{};
    std::array<float,EventCount*EventSize> history{};
    std::array<float,GlobalSize> global{};
    std::array<float,6> mask{};
};
Observation observe(const World&,int agent);
std::vector<uint8_t> snapshot(const World&);
bool restore(World&,const uint8_t*,size_t);
uint64_t hash(const World&);
} // namespace creature

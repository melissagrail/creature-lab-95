#include "creature/sim.hpp"
#include "creature/replay.hpp"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <climits>
using namespace creature;
static int checks=0;
#define CHECK(x) do{checks++;if(!(x)){std::cerr<<"FAIL line "<<__LINE__<<": "#x"\n";std::exit(1);}}while(0)
static World duel(){World w;reset(w,42);w.bodies[0].pos={8*Q,9*Q};w.bodies[1].pos={10*Q,9*Q};return w;}
int main(){
 World a,b;reset(a,42,2);reset(b,42,2);
 for(int i=0;i<900;i++){auto actions=std::array<Action,2>{scripted(a,0),scripted(a,1)};step(a,actions);step(b,actions);CHECK(hash(a)==hash(b));}
 CHECK(a.terminal||a.truncated);auto terminal_hash=hash(a);CHECK(step(a,{}).ticks==0);CHECK(hash(a)==terminal_hash);
 // Replay fork at a live moment, with in-flight effects and populated history.
 reset(a,789,2);for(int i=0;i<40;i++)step(a,{scripted(a,0),scripted(a,1)});
 auto bytes=snapshot(a);CHECK(restore(b,bytes.data(),bytes.size()));CHECK(hash(a)==hash(b));
 for(int i=0;i<60;i++){std::array<Action,2> action{scripted(a,0),scripted(a,1)};step(a,action);step(b,action);CHECK(hash(a)==hash(b));}
 auto before=hash(b);bytes[33]^=1;CHECK(!restore(b,bytes.data(),bytes.size()));CHECK(hash(b)==before);CHECK(!restore(b,bytes.data(),bytes.size()-1));
 // Each seed and every tick remains serializable, even expired projectile slots.
 for(int seed=1;seed<12;seed++){reset(a,seed,seed%3);for(int i=0;i<900&&!a.terminal;i++){step(a,{scripted(a,0),scripted(a,1)});auto s=snapshot(a);CHECK(restore(b,s.data(),s.size()));}}
 a=duel();a.bodies[1].pos={a.bodies[0].pos.x+1400,a.bodies[0].pos.y};
 step(a,{{{0,0,Q,0,1},{0,0,-Q,0,1}}});for(int i=0;i<4;i++)step(a,{});CHECK(a.bodies[0].hp==84);CHECK(a.bodies[1].hp==84);
 // Startup 5 ticks: damage cannot happen early; active contact hits once.
 a=duel();a.bodies[1].pos.x=a.bodies[0].pos.x+1400;
 step(a,{{{0,0,Q,0,1},{}}},1);for(int i=0;i<4;i++)step(a,{},1);CHECK(a.bodies[1].hp==100);step(a,{},1);CHECK(a.bodies[1].hp==84);for(int i=0;i<8;i++)step(a,{},1);CHECK(a.bodies[1].hp==84);
 // Simultaneous knockouts are draws.
 a=duel();a.bodies[1].pos.x=a.bodies[0].pos.x+1400;a.bodies[0].hp=a.bodies[1].hp=16;
 step(a,{{{0,0,Q,0,1},{0,0,-Q,0,1}}});step(a,{});CHECK(a.terminal&&a.winner==-1);
 // Dodge active frames prevent ordinary projectiles, field damage bypasses evasion.
 a=duel();a.bodies[1].move=4;a.bodies[1].age=1;a.projectiles[0]={a.bodies[1].pos,{},5,0,1};auto r=step(a,{},1);CHECK(a.bodies[1].hp==100);CHECK(r.features[1].dodged==1);
 a=duel();a.bodies[1].move=4;a.bodies[1].age=1;a.zones[0]={a.bodies[1].pos,180,0,3,0};step(a,{},1);CHECK(a.bodies[1].hp==96);
 // Wall shields a projectile even at its maximum speed.
 reset(a,1);a.bodies[0].pos={9*Q,5*Q};a.bodies[1].pos={15*Q,5*Q};a.projectiles[0]={{10*Q,5*Q},{800,0},30,0,1};for(int i=0;i<12;i++)step(a,{},1);CHECK(a.bodies[1].hp==100);CHECK(a.projectiles[0].life==0);
 // Energy/cooldown and recovery forbid restarting an action.
 a=duel();a.bodies[0].energy=0;CHECK(action_mask(a,0)[1]==0);step(a,{{{0,0,Q,0,1},{}}});CHECK(a.bodies[0].move==-1);
 a=duel();step(a,{{{0,0,Q,0,2},{}}});int energy=a.bodies[0].energy;step(a,{{{0,0,Q,0,2},{}}});CHECK(a.bodies[0].energy==energy+15);
 // Rain shortens fire persistence; guidance changes no health or movement directly.
 a=duel();b=a;a.rain=800;a.zones[0]=b.zones[0]={{4*Q,4*Q},180,0,3,0};step(a,{});step(b,{});CHECK(a.zones[0].life<b.zones[0].life);
 a=duel();Vec p=a.bodies[0].pos;command(a,0,Retreat);CHECK(a.bodies[0].pos.x==p.x&&a.bodies[0].hp==100);CHECK(observe(a,0).self[15]>0);
 // Private opponent variables do not enter actor tensors.
 a=duel();auto o1=observe(a,0);a.bodies[1].energy=123;a.bodies[1].cooldown[0]=17;auto o2=observe(a,0);CHECK(o1.self==o2.self);CHECK(o1.entities==o2.entities);CHECK(o1.history==o2.history);command(a,1,Retreat);CHECK(observe(a,0).history==o1.history);
 // Extreme external actions are clamped before arithmetic, no out-of-bounds moves.
 reset(a,42);for(int i=0;i<100;i++)step(a,{{{INT_MAX,INT_MIN,INT_MIN,INT_MAX,INT_MIN},{}}});CHECK(a.bodies[0].pos.x<=24*Q&&a.bodies[0].pos.y>=a.bodies[0].radius);
 reset(a,1);a.tick=MaxTicks-1;step(a,{});CHECK(a.truncated&&!a.terminal);
 // Replay actions, commands and praise roundtrip; reject tampered end hash.
 Replay tape;reset(tape.initial,42);a=tape.initial;
 for(int k=0;k<40;k++){ReplayFrame f;f.actions={scripted(a,0),scripted(a,1)};if(k==10){f.guidance[0]=Retreat;f.feedback=1;}replay_step(a,f);f.expected_hash=hash(a);tape.frames.push_back(f);}
 CHECK(save_replay(tape,"test-replay.tmp"));Replay loaded;CHECK(load_replay(loaded,"test-replay.tmp"));CHECK(loaded.frames.size()==40&&loaded.frames[10].feedback==1);
 tape.frames.back().expected_hash^=1;CHECK(save_replay(tape,"test-replay.tmp"));CHECK(!load_replay(loaded,"test-replay.tmp"));std::remove("test-replay.tmp");
 std::cout<<checks<<" checks passed; golden ";reset(a,77,2);for(int i=0;i<100;i++)step(a,{scripted(a,0),scripted(a,1)});CHECK(hash(a)==0xb92b46e8c4a2443cull);std::cout<<std::hex<<hash(a)<<"\n";
}

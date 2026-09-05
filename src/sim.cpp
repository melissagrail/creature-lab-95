#include "creature/sim.hpp"
#include <algorithm>
#include <limits>
#include <type_traits>

namespace creature {
const std::array<Move,5> Moves{{
    {"QUICK CLAW",Melee,5,3,9,21,80,16,1800,800,0,190,0,0,0},
    {"EMBER BOLT",Bolt,10,1,10,33,140,19,14000,220,430,90,90,0,0},
    {"THUNDER LUNGE",Lunge,7,6,13,48,220,23,1700,650,340,320,0,0,0},
    {"CINDER PATCH",Field,14,1,12,90,260,4,4600,1900,0,0,60,90,1},
    {"DODGE",Evade,1,6,8,30,240,0,0,0,410,0,0,0,0}
}};
Vec operator+(Vec a,Vec b){return {a.x+b.x,a.y+b.y};}
Vec operator-(Vec a,Vec b){return {a.x-b.x,a.y-b.y};}
Vec scale(Vec v,int n,int d){return {int32_t(int64_t(v.x)*n/d),int32_t(int64_t(v.y)*n/d)};}
static int64_t dot(Vec a,Vec b){return int64_t(a.x)*b.x+int64_t(a.y)*b.y;}
static uint64_t isqrt(uint64_t n){uint64_t r=0,b=uint64_t(1)<<62;while(b>n)b>>=2;while(b){if(n>=r+b){n-=r+b;r=(r>>1)+b;}else r>>=1;b>>=2;}return r;}
int length(Vec v){return int(isqrt(uint64_t(dot(v,v))));}
Vec unit(Vec v){int n=length(v);return n?scale(v,Q,n):Vec{Q,0};}
static uint32_t random(World& w){uint32_t x=w.rng;x^=x<<13;x^=x>>17;x^=x<<5;return w.rng=x;}
static void event(World& w,int kind,int actor,int target,int move,int amount,Vec pos){w.events[w.event_head]={w.tick,kind,actor,target,move,amount,pos};w.event_head=(w.event_head+1)%HistoryCount;w.event_count=std::min(HistoryCount,w.event_count+1);}
void reset(World& w,uint32_t seed,int weather){w=World{};w.rng=seed?seed:1;int j=int(random(w)%2049)-1024;w.bodies[0].pos={5*Q,9*Q+j};w.bodies[1].pos={19*Q,9*Q-j};w.bodies[1].aim=w.bodies[1].locked={-Q,0};w.obstacles={Obstacle{{12*Q,5*Q},1200},Obstacle{{12*Q,13*Q},1200}};w.rain=weather==2?800:0;w.wind=weather?int(random(w)%9)+5:0;}
Phase phase(const Body& b){if(b.move<0)return Idle;const auto&m=Moves[b.move];if(b.age<m.startup)return Startup;if(b.age<m.startup+m.active)return Active;return Recovery;}
void command(World&w,int a,int g){if(a<0||a>1||g<0||g>3||w.terminal||w.truncated)return;auto&b=w.bodies[a];b.guidance=g;b.guidance_age=0;event(w,Command,a,a,-1,g,b.pos);}
std::array<int32_t,6> action_mask(const World&w,int a){std::array<int32_t,6> r{1,0,0,0,0,0};if(a<0||a>1||w.terminal||w.truncated)return r;const auto&b=w.bodies[a];for(int i=0;i<5;i++)r[i+1]=b.move<0&&!b.stun&&b.hp>0&&b.energy>=Moves[i].cost&&!b.cooldown[i];return r;}
static bool overlap(Vec a,int ar,Vec b,int br){auto d=a-b;return dot(d,d)<=int64_t(ar+br)*(ar+br);}
// Bounded substeps (<= 1/16 unit) avoid tunneling; geometry uses only integers.
static Vec terrain(const World&w,Vec p,int radius){p.x=std::clamp(p.x,radius,24*Q-radius);p.y=std::clamp(p.y,radius,18*Q-radius);for(const auto&o:w.obstacles){Vec d=p-o.pos;int n=length(d),r=radius+o.radius;if(n<r)p=o.pos+scale(unit(d),r+1);}return p;}
Vec target_point(const World&w,int agent,int move){if(agent<0||agent>1||move<0||move>4)return {};auto&b=w.bodies[agent];auto&m=Moves[move];Vec p=b.pos+scale(b.locked,m.range);return m.kind==Field?terrain(w,p,m.radius):p;}
static Vec move_body(const World&w,Vec p,Vec delta,int radius){int n=std::max(1,(length(delta)+63)/64);Vec prev{};for(int i=1;i<=n;i++){Vec off=scale(delta,i,n);p=terrain(w,p+off-prev,radius);prev=off;}return p;}
static bool clear_line(const World&w,Vec a,Vec b,int radius=0){int n=std::max(1,(length(b-a)+63)/64);for(int i=0;i<=n;i++){Vec p=a+scale(b-a,i,n);for(auto&o:w.obstacles)if(overlap(p,radius,o.pos,o.radius))return false;}return true;}
struct Pending {int source,target,move,damage;Vec direction;};
static void add_features(StepResult&out,const StepResult&r){out.ticks+=r.ticks;for(int i=0;i<2;i++){auto&a=out.features[i];auto&b=r.features[i];a.dealt+=b.dealt;a.taken+=b.taken;a.dodged+=b.dodged;a.interrupts+=b.interrupts;a.ko+=b.ko;a.death+=b.death;}}
static StepResult tick(World&w,const std::array<Action,2>&actions,bool trigger){
    StepResult result{};if(w.terminal||w.truncated)return result;result.ticks=1;
    std::array<Pending,64> pending{};int count=0;
    auto queue=[&](int s,int t,int m,int damage,Vec dir){if(count<int(pending.size()))pending[count++]={s,t,m,damage,dir};else{w.overflow++;event(w,Overflow,s,t,m,0,{});}};
    // Readiness, input sanitization, then action start for BOTH actors before combat.
    for(int i=0;i<2;i++){
        auto&b=w.bodies[i];b.guidance_age=std::min(900,b.guidance_age+1);if(b.guidance_age==900)b.guidance=Free;
        for(auto&c:b.cooldown)if(c)c--;if(b.stun)b.stun--;if(b.haste)b.haste--;
        b.energy=std::min(1000,b.energy+5);
        Vec aim{std::clamp(actions[i].ax,-Q,Q),std::clamp(actions[i].ay,-Q,Q)};
        if(b.move<0&&(aim.x||aim.y))b.aim=unit(aim);
        int m=actions[i].ability>=1&&actions[i].ability<=5?actions[i].ability-1:-1;
        if(trigger&&m>=0&&m<5&&action_mask(w,i)[m+1]){b.move=m;b.age=0;b.hit_mask=0;b.locked=b.aim;b.energy-=Moves[m].cost;b.cooldown[m]=Moves[m].cooldown;event(w,Started,i,i,m,0,b.pos);}
    }
    for(int i=0;i<2;i++){
        auto&b=w.bodies[i];Vec input{std::clamp(actions[i].mx,-Q,Q),std::clamp(actions[i].my,-Q,Q)};if(length(input)>Q)input=unit(input);
        int speed=b.haste?195:156;if(phase(b)==Startup)speed=speed*2/3;if(b.stun)speed=0;
        Vec target=scale(input,speed);int traction=w.wetness>400?5:3;b.vel=b.vel+scale(target-b.vel,1,traction);
        if(b.move>=0&&phase(b)==Active&&(Moves[b.move].kind==Lunge||Moves[b.move].kind==Evade))b.vel=scale(b.locked,Moves[b.move].speed);
        Vec old=b.pos;b.pos=move_body(w,b.pos,b.vel,b.radius);b.vel=b.pos-old;
    }
    // Symmetric separation; fixed iteration count, deterministic tie direction.
    for(int n=0;n<3;n++){auto&a=w.bodies[0];auto&b=w.bodies[1];Vec d=b.pos-a.pos;int depth=a.radius+b.radius-length(d);if(depth>0){Vec push=scale(unit(d),(depth+2)/2);a.pos=terrain(w,a.pos-push,a.radius);b.pos=terrain(w,b.pos+push,b.radius);}}
    for(int i=0;i<2;i++){
        auto&b=w.bodies[i];if(b.move<0)continue;const auto&m=Moves[b.move];
        if(b.age==m.startup){event(w,Released,i,i,b.move,0,b.pos);
            if(m.haste)b.haste=std::max(b.haste,m.haste);
            if(m.kind==Bolt){bool allocated=false;for(auto&p:w.projectiles)if(!p.life){p={b.pos,scale(b.locked,m.speed),std::max(1,m.range/m.speed),i,b.move};allocated=true;break;}if(!allocated){w.overflow++;event(w,Overflow,i,i,b.move,0,b.pos);}}
            if(m.kind==Field){bool allocated=false;Vec p=target_point(w,i,b.move);for(auto&z:w.zones)if(!z.life){z={p,180,i,b.move,0};allocated=true;break;}if(!allocated){w.overflow++;event(w,Overflow,i,i,b.move,0,b.pos);}}
        }
        if(phase(b)==Active&&(m.kind==Melee||m.kind==Lunge)&&!(b.hit_mask&(1<<(1-i)))){
            auto&t=w.bodies[1-i];Vec center=b.pos+scale(b.locked,m.range/2);
            if(overlap(center,m.radius,t.pos,t.radius)&&clear_line(w,b.pos,t.pos)){queue(i,1-i,b.move,m.damage,b.locked);b.hit_mask|=1<<(1-i);}
        }
    }
    for(auto&p:w.projectiles)if(p.life){const auto&m=Moves[p.move];p.vel.x=std::clamp(p.vel.x+w.wind,-800,800);Vec start=p.pos;int n=std::max(1,(length(p.vel)+63)/64);
        for(int s=1;s<=n&&p.life;s++){Vec next=start+scale(p.vel,s,n);bool wall=next.x<m.radius||next.x>24*Q-m.radius||next.y<m.radius||next.y>18*Q-m.radius;for(auto&o:w.obstacles)wall=wall||overlap(next,m.radius,o.pos,o.radius);p.pos=next;if(wall){p.life=0;break;}auto&t=w.bodies[1-p.owner];if(overlap(next,m.radius,t.pos,t.radius)){queue(p.owner,1-p.owner,p.move,m.damage,unit(p.vel));p.life=0;}}
        if(p.life)p.life--;
    }
    for(auto&z:w.zones)if(z.life){const auto&m=Moves[z.move];if(z.age%15==0)for(int i=0;i<2;i++)if(overlap(z.pos,m.radius,w.bodies[i].pos,w.bodies[i].radius))queue(z.owner,i,z.move,m.damage,unit(w.bodies[i].pos-z.pos));z.age++;z.life=std::max(0,z.life-(w.rain?2:1));}
    // Capture evasive phase before resolving any hits; simultaneous attacks can trade.
    std::array<bool,2> evasive{};for(int i=0;i<2;i++)evasive[i]=w.bodies[i].move==4&&phase(w.bodies[i])==Active;
    for(int n=0;n<count;n++){auto&p=pending[n];auto&t=w.bodies[p.target];auto&m=Moves[p.move];if(evasive[p.target]&&!m.pierces_evasion){result.features[p.target].dodged++;event(w,Dodged,p.target,p.source,p.move,0,t.pos);continue;}
        int damage=std::min(t.hp,p.damage);t.hp-=damage;result.features[p.target].taken+=damage;result.features[p.source].dealt+=damage;
        event(w,Hit,p.source,p.target,p.move,damage,t.pos);
        if(m.burn){t.burn=std::max(t.burn,m.burn);t.burn_owner=p.source;}
        if(m.impulse){t.pos=move_body(w,t.pos,scale(p.direction,m.impulse),t.radius);}
        if(m.kind==Lunge&&t.move>=0&&phase(t)==Startup){event(w,Interrupted,p.source,p.target,t.move,0,t.pos);t.move=-1;t.age=0;t.stun=6;result.features[p.source].interrupts++;}
    }
    for(int i=0;i<2;i++){auto&b=w.bodies[i];if(b.burn){if(w.tick%30==29){int d=std::min(b.hp,2);b.hp-=d;result.features[i].taken+=d;result.features[b.burn_owner].dealt+=d;event(w,Hit,b.burn_owner,i,1,d,b.pos);}b.burn=std::max(0,b.burn-(w.rain?2:1));}
        if(b.move>=0){const auto&m=Moves[b.move];b.age++;if(b.age>=m.startup+m.active+m.recovery){event(w,Ended,i,i,b.move,0,b.pos);b.move=-1;b.age=0;b.hit_mask=0;}}
        if(b.hp==0){event(w,Knockout,1-i,i,-1,0,b.pos);result.features[i].death++;result.features[1-i].ko++;w.terminal=1;}
    }
    w.wetness=std::clamp(w.wetness+(w.rain?3:-1),0,1000);w.tick++;
    if(w.terminal)w.winner=w.bodies[0].hp==w.bodies[1].hp?-1:(w.bodies[0].hp>0?0:1);
    else if(w.tick>=MaxTicks)w.truncated=1;
    return result;
}
StepResult step(World&w,const std::array<Action,2>&a,int ticks){StepResult r;for(int t=0;t<std::clamp(ticks,0,DecisionTicks);t++)add_features(r,tick(w,a,t==0));return r;}
Action scripted(const World&w,int i){if(i<0||i>1)return {};const auto&b=w.bodies[i];const auto&e=w.bodies[1-i];Vec d=e.pos-b.pos;int distance=length(d);Vec aim=unit(d+scale(e.vel,5,1));Vec movement=unit(d);int desired=b.guidance==Retreat?8500:4500;if(b.guidance==Attack)desired=2000;
    if(distance<desired)movement=scale(movement,-Q);else if(distance<desired+1000)movement={-movement.y,movement.x};
    for(const auto&o:w.obstacles)if(length(o.pos-b.pos)<o.radius+b.radius+1300){Vec away=unit(b.pos-o.pos);movement=unit(movement+scale(away,2,1));}
    int ability=0;auto mask=action_mask(w,i);
    if(b.guidance!=Conserve){if(distance<2400&&mask[1])ability=1;else if(distance<4300&&mask[3])ability=3;else if(distance<6500&&mask[4]&&w.tick%90<15)ability=4;else if(mask[2]&&clear_line(w,b.pos,e.pos))ability=2;}
    bool danger=e.move>=0&&phase(e)==Startup&&distance<3500;
    for(auto&p:w.projectiles)if(p.life&&p.owner!=i&&length(p.pos-b.pos)<2800)danger=true;
    if(danger&&mask[5]){ability=5;aim=unit(Vec{-d.y,d.x});}
    return {movement.x,movement.y,aim.x,aim.y,ability};
}
Observation observe(const World&w,int i){Observation o{};if(i<0||i>1)return o;auto&b=w.bodies[i];auto&e=w.bodies[1-i];
    auto xy=[&](Vec v){return std::array<float,2>{float(dot(v,b.aim))/(Q*24*Q),float(int64_t(v.y)*b.aim.x-int64_t(v.x)*b.aim.y)/(Q*24*Q)};};
    o.self={float(b.hp)/100,float(b.energy)/1000,float(b.vel.x)/512,float(b.vel.y)/512,float(b.aim.x)/Q,float(b.aim.y)/Q,float(b.move+1)/5,float(phase(b))/3,float(b.age)/90,float(b.stun)/30,float(b.burn)/90,float(b.haste)/90,float(b.radius)/Q,float(b.pos.x)/(24*Q),float(b.pos.y)/(18*Q),float(b.guidance)/3,float(b.guidance_age)/900,0,0,0,0,0,0,0};
    for(int j=0;j<5;j++)o.self[17+j]=float(b.cooldown[j])/Moves[j].cooldown;
    auto entity=[&](int row,int kind,Vec pos,Vec vel,int radius,int team,int life,int move,int ph,int age,float hp,int burn,int haste){float*p=o.entities.data()+row*EntitySize;auto r=xy(pos-b.pos),v=xy(vel);p[0]=float(kind)/4;p[1]=r[0];p[2]=r[1];p[3]=v[0]*30;p[4]=v[1]*30;p[5]=float(radius)/(24*Q);p[6]=float(team);p[7]=float(life)/180;p[8]=float(move+1)/5;p[9]=float(ph)/3;p[10]=float(age)/90;p[11]=hp;p[12]=float(burn)/90;Vec facing=kind==1?(e.move>=0?e.locked:e.aim):(kind==3?unit(vel):Vec{});p[13]=float(dot(facing,b.aim))/(Q*Q);p[14]=float(int64_t(facing.y)*b.aim.x-int64_t(facing.x)*b.aim.y)/(Q*Q);p[15]=1;(void)haste;};
    entity(0,1,e.pos,e.vel,e.radius,-1,0,e.move,phase(e),e.age,float(e.hp)/100,e.burn,e.haste);
    for(int j=0;j<2;j++)entity(1+j,2,w.obstacles[j].pos,{},w.obstacles[j].radius,0,0,-1,0,0,0,0,0);
    for(int j=0;j<ProjectileCount;j++){auto&p=w.projectiles[j];if(p.life)entity(3+j,3,p.pos,p.vel,Moves[p.move].radius,p.owner==i?1:-1,p.life,p.move,0,0,0,0,0);}
    for(int j=0;j<ZoneCount;j++){auto&z=w.zones[j];if(z.life)entity(19+j,4,z.pos,{},Moves[z.move].radius,z.owner==i?1:-1,z.life,z.move,0,z.age,0,0,0);}
    for(int j=0;j<5;j++){auto&m=Moves[j];float*p=o.moves.data()+j*MoveSize;const float values[]={float(m.kind)/4,float(m.startup)/30,float(m.active)/30,float(m.recovery)/30,float(m.cooldown)/90,float(m.cost)/1000,float(m.damage)/100,float(m.range)/(24*Q),float(m.radius)/(24*Q),float(m.speed)/512,float(m.impulse)/512,float(m.burn)/90,float(m.haste)/90,float(m.pierces_evasion),1,0};std::copy(std::begin(values),std::end(values),p);}
    int visible=0;
    for(int j=0;j<w.event_count&&visible<EventCount;j++){
        auto&e2=w.events[(w.event_head-1-j+HistoryCount)%HistoryCount];
        if(e2.kind==Command&&e2.actor!=i)continue;
        float*p=o.history.data()+visible++*EventSize;p[0]=float(w.tick-e2.tick)/90;p[1]=float(e2.kind)/9;p[2]=e2.actor==i?1:-1;p[3]=e2.target==i?1:-1;p[4]=float(e2.move+1)/5;p[5]=float(e2.amount)/100;p[6]=1;p[7]=1;
    }
    o.global={float(w.tick)/MaxTicks,float(w.rain)/1000,float(w.wind)/16,float(w.wetness)/1000,float(w.terminal),float(w.truncated),0,0};auto mask=action_mask(w,i);for(int j=0;j<6;j++)o.mask[j]=float(mask[j]);return o;
}
// One explicit field visitor is shared by encoder/decoder: no ABI padding, pointers,
// raw struct dumps, host endianness, or library-dependent serialization.
template<class F> static void fields(World&w,F f){f(w.rng);f(w.tick);f(w.terminal);f(w.truncated);f(w.winner);f(w.rain);f(w.wind);f(w.wetness);f(w.event_head);f(w.event_count);f(w.overflow);auto vec=[&](Vec&v){f(v.x);f(v.y);};for(auto&b:w.bodies){vec(b.pos);vec(b.vel);vec(b.aim);vec(b.locked);f(b.hp);f(b.energy);f(b.radius);f(b.move);f(b.age);f(b.hit_mask);f(b.stun);f(b.burn);f(b.burn_owner);f(b.haste);for(auto&c:b.cooldown)f(c);f(b.guidance);f(b.guidance_age);}for(auto&p:w.projectiles){vec(p.pos);vec(p.vel);f(p.life);f(p.owner);f(p.move);}for(auto&z:w.zones){vec(z.pos);f(z.life);f(z.owner);f(z.move);f(z.age);}for(auto&o:w.obstacles){vec(o.pos);f(o.radius);}for(auto&e:w.events){f(e.tick);f(e.kind);f(e.actor);f(e.target);f(e.move);f(e.amount);vec(e.pos);}}
static uint64_t checksum(const uint8_t*p,size_t n){uint64_t h=14695981039346656037ull;for(size_t i=0;i<n;i++){h^=p[i];h*=1099511628211ull;}return h;}
std::vector<uint8_t> snapshot(const World&world){World w=world;std::vector<uint8_t> out{'C','R','L','B'};auto emit=[&](auto value){uint32_t v=uint32_t(value);for(int j=0;j<4;j++)out.push_back(uint8_t(v>>(j*8)));};emit(RulesVersion);fields(w,emit);uint64_t h=checksum(out.data(),out.size());for(int j=0;j<8;j++)out.push_back(uint8_t(h>>(8*j)));return out;}
static bool valid(const World&w){if(w.overflow<0||w.overflow>100000||!w.rng||w.tick<0||w.tick>MaxTicks||w.event_head<0||w.event_head>=HistoryCount||w.event_count<0||w.event_count>HistoryCount||w.wind<-16||w.wind>16||w.rain<0||w.rain>1000||w.wetness<0||w.wetness>1000||w.terminal<0||w.terminal>1||w.truncated<0||w.truncated>1||w.winner<-1||w.winner>1)return false;auto pos=[](Vec v){return v.x>=-Q&&v.x<=25*Q&&v.y>=-Q&&v.y<=19*Q;};auto vel=[](Vec v){return v.x>=-2*Q&&v.x<=2*Q&&v.y>=-2*Q&&v.y<=2*Q;};for(auto&b:w.bodies){if(b.hit_mask<0||b.hit_mask>3||(b.move==-1&&b.age!=0)||(b.move>=0&&b.move<5&&b.age>=Moves[b.move].startup+Moves[b.move].active+Moves[b.move].recovery))return false;if(!pos(b.pos)||!vel(b.vel)||!vel(b.aim)||!vel(b.locked)||b.hp<0||b.hp>100||b.energy<0||b.energy>1000||b.radius!=460||b.move<-1||b.move>4||b.age<0||b.age>90||b.burn<0||b.burn>90||b.burn_owner<0||b.burn_owner>1||b.stun<0||b.stun>30||b.haste<0||b.haste>90||b.guidance<0||b.guidance>3||b.guidance_age<0||b.guidance_age>900)return false;for(auto c:b.cooldown)if(c<0||c>90)return false;}for(auto&p:w.projectiles)if(!pos(p.pos)||!vel(p.vel)||p.life<0||p.life>40||p.owner<0||p.owner>1||p.move!=1)return false;for(auto&z:w.zones)if(!pos(z.pos)||z.life<0||z.life>180||z.owner<0||z.owner>1||z.move!=3||z.age<0||z.age>180)return false;for(auto&o:w.obstacles)if(!pos(o.pos)||o.radius<1||o.radius>2*Q)return false;for(auto&e:w.events)if(e.tick<0||e.tick>MaxTicks||e.kind<0||e.kind>9||e.actor<0||e.actor>1||e.target<0||e.target>1||e.move<-1||e.move>4||!pos(e.pos))return false;return true;}
bool restore(World&w,const uint8_t*p,size_t n){if(!p||n!=snapshot(World{}).size()||p[0]!='C'||p[1]!='R'||p[2]!='L'||p[3]!='B')return false;size_t off=4;auto read=[&](){uint32_t v=0;for(int j=0;j<4;j++)v|=uint32_t(p[off++])<<(8*j);return v;};if(read()!=RulesVersion)return false;uint64_t h=0;for(int j=0;j<8;j++)h|=uint64_t(p[n-8+j])<<(8*j);if(h!=checksum(p,n-8))return false;World candidate;fields(candidate,[&](auto&v){uint32_t raw=read();using T=std::decay_t<decltype(v)>;if constexpr(std::is_signed_v<T>)v=raw<=uint32_t(INT32_MAX)?int32_t(raw):int32_t(int64_t(raw)-4294967296ll);else v=raw;});if(!valid(candidate))return false;w=candidate;return true;}
uint64_t hash(const World&w){auto bytes=snapshot(w);return checksum(bytes.data(),bytes.size()-8);}
} // namespace creature

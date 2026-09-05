#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "creature/sim.hpp"
#include "creature/replay.hpp"
#include "font.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace creature;
namespace {
SDL_Renderer*r=nullptr;
const SDL_Color black{25,28,29,255},white{255,255,244,255},grey{192,192,192,255},dark{100,100,100,255},blue{27,75,158,255},orange{195,75,20,255},yellow{245,190,30,255},green{20,118,79,255};
void color(SDL_Color c){SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a);}
void rect(int x,int y,int w,int h,SDL_Color c){color(c);SDL_Rect s{x,y,w,h};SDL_RenderFillRect(r,&s);}
void line(int x,int y,int xx,int yy,SDL_Color c){color(c);SDL_RenderDrawLine(r,x,y,xx,yy);}
void panel(int x,int y,int w,int h,bool inset=false){rect(x,y,w,h,grey);auto a=inset?dark:white,b=inset?white:dark;line(x,y,x+w-1,y,a);line(x,y,x,y+h-1,a);line(x,y+h-1,x+w-1,y+h-1,b);line(x+w-1,y,x+w-1,y+h-1,b);}
void label(int x,int y,std::string s,SDL_Color c=black,int scale=2){text(r,x,y,s,c,scale);}
void circle(int x,int y,int radius,SDL_Color c,bool fill=false){color(c);if(fill){for(int yy=-radius;yy<=radius;yy++){int half=int(std::sqrt(double(radius*radius-yy*yy)));SDL_RenderDrawLine(r,x-half,y+yy,x+half,y+yy);}return;}for(int i=0;i<96;i++){double a=i*6.283185307/96,b=(i+1)*6.283185307/96;SDL_RenderDrawLine(r,x+int(std::cos(a)*radius),y+int(std::sin(a)*radius),x+int(std::cos(b)*radius),y+int(std::sin(b)*radius));}}
constexpr int AX=24,AY=158,S=29;
int sx(int x){return AX+x*S/Q;}int sy(int y){return AY+y*S/Q;}
void worldcircle(Vec p,int radius,SDL_Color c,bool fill=false){circle(sx(p.x),sy(p.y),std::max(1,radius*S/Q),c,fill);}
const char* phase_name(Phase p){return p==Idle?"READY":p==Startup?"WINDUP":p==Active?"ACTIVE":"RECOVERY";}
std::string num(int n){return std::to_string(n);}
struct Button{SDL_Rect box;std::string label;int id;};
std::vector<Button> buttons;
void button(int id,int x,int y,int w,std::string name,bool selected=false){buttons.push_back({{x,y,w,28},name,id});panel(x,y,w,28,selected);label(x+8,y+8,name,selected?blue:black,1);}
void meter(int x,int y,int w,int value,int max,SDL_Color c){panel(x,y,w,12,true);rect(x+2,y+2,(w-4)*std::clamp(value,0,max)/max,8,c);}
}
int main(int argc,char**argv){
 SDL_SetMainReady();if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){std::cerr<<SDL_GetError()<<'\n';return 1;}
 auto*window=SDL_CreateWindow("Creature Lab 95 - deterministic combat workbench",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1100,780,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
 if(!window){std::cerr<<SDL_GetError()<<'\n';return 1;}r=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);if(!r)r=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);if(!r){std::cerr<<SDL_GetError()<<'\n';return 1;}
 SDL_RenderSetLogicalSize(r,1100,780);SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
 bool run=true,paused=false,manual=false,playback=false;int weather=0,speed=1,ability=0,pending_guidance=-1,feedback=0;uint32_t seed=42;size_t cursor=0;World w;reset(w,seed,weather);Replay tape{w,{}};std::vector<uint8_t> saved;std::string note="SCRIPTED BRAINS. NO TRAINING OR XP YET.";
 std::string captures="captures";if(std::string(argv[0]).find(".app/Contents/MacOS/")!=std::string::npos)captures=(std::filesystem::path(argv[0]).parent_path().parent_path().parent_path().parent_path().parent_path()/"captures").string();int frames_limit=0;for(int i=1;i<argc;i++){std::string a=argv[i];if(a=="--frames"&&i+1<argc)frames_limit=std::stoi(argv[++i]);else if(a=="--captures"&&i+1<argc)captures=argv[++i];}
 std::error_code file_error;std::filesystem::create_directories(captures,file_error);if(file_error){std::cerr<<"Cannot create captures directory: "<<file_error.message()<<"\n";SDL_DestroyRenderer(r);SDL_DestroyWindow(window);SDL_Quit();return 1;}
 auto restart=[&](){reset(w,seed,weather);tape={w,{}};playback=false;cursor=0;ability=0;pending_guidance=-1;feedback=0;note="NEW EPISODE / SEED "+num(seed);};
 auto act=[&](int id){switch(id){
 case 0:run=false;break;
 case 1:paused=!paused;break;
 case 2:paused=true;break;
 case 3:restart();break;
 case 4:manual=!manual;ability=0;note=manual?"WASD MOVE / MOUSE AIM / 1-4 MOVES / SPACE DODGE":"BOTH CREATURES USE SCRIPTED POLICIES";break;
 case 5:weather=(weather+1)%3;restart();break;
 case 6:saved=snapshot(w);{std::ofstream f(captures+"/snapshot.crs",std::ios::binary);f.write(reinterpret_cast<const char*>(saved.data()),saved.size());note=f?"SNAPSHOT SAVED. F9 TO FORK FROM HERE.":"SNAPSHOT WRITE FAILED";}break;
 case 7:{if(saved.empty()){std::ifstream f(captures+"/snapshot.crs",std::ios::binary|std::ios::ate);if(f&&f.tellg()==std::streampos(snapshot(World{}).size())){saved.resize(size_t(f.tellg()));f.seekg(0);f.read(reinterpret_cast<char*>(saved.data()),saved.size());}}if(restore(w,saved.data(),saved.size())){tape={w,{}};playback=false;paused=true;ability=0;pending_guidance=-1;feedback=0;note="FORK RESTORED / NEW REPLAY BRANCH";}else note="NO VALID SNAPSHOT";break;}
 case 8:note=save_replay(tape,captures+"/battle.crr")?"REPLAY SAVED WITH ACTIONS, FEEDBACK AND HASHES":"REPLAY SAVE FAILED";break;
 case 9:{Replay loaded;if(load_replay(loaded,captures+"/battle.crr")){tape=std::move(loaded);w=tape.initial;cursor=0;playback=true;paused=false;note="REPLAY VERIFIED. PLAYING RECORDED ACTIONS.";}else note="REPLAY MISSING, INCOMPATIBLE OR CORRUPT";break;}
 case 10:speed=speed==1?4:1;break;
 case 11:pending_guidance=Attack;note="GUIDANCE: ATTACK (POLICY INPUT)";break;
 case 12:pending_guidance=Retreat;note="GUIDANCE: RETREAT (POLICY INPUT)";break;
 case 13:pending_guidance=Conserve;note="GUIDANCE: SAVE ENERGY (POLICY INPUT)";break;
 case 14:pending_guidance=Free;note="GUIDANCE: FREE";break;
 case 15:feedback=1;note="PRAISE QUEUED FOR EXPERIENCE LOG. NO STAT CHANGE.";break;
 case 16:feedback=-1;note="CORRECTION QUEUED FOR EXPERIENCE LOG.";break;
 case 17:seed++;restart();break;
 }};
 auto advance=[&](){if(w.terminal||w.truncated)return;if(playback){if(cursor>=tape.frames.size()){paused=true;note="REPLAY COMPLETE / ALL HASHES MATCH";return;}auto&f=tape.frames[cursor++];replay_step(w,f);if(hash(w)!=f.expected_hash){paused=true;note="REPLAY HASH MISMATCH";}return;}
 ReplayFrame f;f.guidance[0]=pending_guidance;f.feedback=feedback;if(pending_guidance>=0)command(w,0,pending_guidance);pending_guidance=-1;feedback=0;
 f.actions={scripted(w,0),scripted(w,1)};
 if(manual){const Uint8*keys=SDL_GetKeyboardState(nullptr);int mx,my;SDL_GetMouseState(&mx,&my);float lx,ly;SDL_RenderWindowToLogical(r,mx,my,&lx,&ly);Vec aim{int((lx-AX)*Q/S)-w.bodies[0].pos.x,int((ly-AY)*Q/S)-w.bodies[0].pos.y};aim=unit(aim);f.actions[0]={(keys[SDL_SCANCODE_D]-keys[SDL_SCANCODE_A])*Q,(keys[SDL_SCANCODE_S]-keys[SDL_SCANCODE_W])*Q,aim.x,aim.y,ability};ability=0;}
 step(w,f.actions);f.expected_hash=hash(w);tape.frames.push_back(f);
 };
 uint64_t last=SDL_GetPerformanceCounter();double accumulator=0;int rendered=0;
 while(run){bool single=false;SDL_Event e;while(SDL_PollEvent(&e)){if(e.type==SDL_QUIT)run=false;if(e.type==SDL_WINDOWEVENT&&e.window.event==SDL_WINDOWEVENT_FOCUS_LOST){paused=true;ability=0;}
 if(e.type==SDL_KEYDOWN&&!e.key.repeat){auto k=e.key.keysym.sym;if(k==SDLK_ESCAPE)run=false;else if(k==SDLK_p)act(1);else if(k==SDLK_n){act(2);single=true;}else if(k==SDLK_r)act(3);else if(k==SDLK_m)act(4);else if(k==SDLK_F5)act(6);else if(k==SDLK_F9)act(7);else if(k>=SDLK_1&&k<=SDLK_4)ability=int(k-SDLK_1)+1;else if(k==SDLK_SPACE)ability=5;}
 if(e.type==SDL_MOUSEBUTTONDOWN&&e.button.button==SDL_BUTTON_LEFT){float x=float(e.button.x),y=float(e.button.y);for(auto&b:buttons)if(x>=b.box.x&&x<b.box.x+b.box.w&&y>=b.box.y&&y<b.box.y+b.box.h){act(b.id);if(b.id==2)single=true;break;}}
 }
 uint64_t now=SDL_GetPerformanceCounter();double delta=double(now-last)/SDL_GetPerformanceFrequency();last=now;if(!paused)accumulator+=std::min(delta,0.25)*speed;else accumulator=0;while(accumulator>=0.1){advance();accumulator-=0.1;}if(single)advance();
 buttons.clear();rect(0,0,1100,780,{0,112,112,255});panel(10,10,1080,760);rect(14,14,1072,26,{0,0,128,255});label(22,20,"CREATURE LAB 95",white,2);label(720,23,"COMBAT SIMULATOR / BUILD 0.1",white,1);panel(1058,18,22,18);buttons.push_back({{1058,18,22,18},"X",0});label(1064,22,"X",black,1);
 label(24,50,"FILE   SIMULATION   INSPECT   HELP",black,1);label(768,50,"30 HZ WORLD / 10 HZ BRAIN",dark,1);
 button(1,24,70,86,paused?"RESUME [P]":"PAUSE [P]",paused);button(2,116,70,86,"STEP [N]");button(3,208,70,86,"RESET [R]");button(4,300,70,130,manual?"HUMAN + BOT [M]":"BOT + BOT [M]",manual);button(5,436,70,126,weather==0?"WEATHER: CLEAR":weather==1?"WEATHER: WIND":"WEATHER: RAIN");button(10,568,70,68,"SPEED "+num(speed)+"X");button(17,642,70,78,"NEW SEED");
 button(6,744,70,96,"SNAPSHOT F5");button(7,846,70,96,"FORK F9");button(8,948,70,116,"SAVE REPLAY");
 panel(24,108,696,36,true);label(34,120,"ARENA 01 / TRAINING GROUND",black,1);label(425,120,"SEED "+num(seed)+"   TICK "+num(w.tick)+" / 2700",black,1);
 panel(22,156,700,526,true);SDL_Rect arena_clip{AX,AY,24*S,18*S};SDL_RenderSetClipRect(r,&arena_clip);rect(AX,AY,24*S,18*S,{227,228,207,255});
 for(int x=0;x<=24;x++)line(AX+x*S,AY,AX+x*S,AY+18*S,{211,212,194,255});for(int y=0;y<=18;y++)line(AX,AY+y*S,AX+24*S,AY+y*S,{211,212,194,255});
 if(w.wetness){rect(AX,AY,24*S,18*S,{80,137,168,uint8_t(w.wetness/14)});for(int j=0;j<24;j++){int x=AX+(j*83+w.tick*2)%690,y=AY+(j*71+w.tick*4)%515;line(x,y,x+3,y+7,{120,155,173,255});}}
 for(auto&z:w.zones)if(z.life){worldcircle(z.pos,Moves[z.move].radius,{210,100,30,65},true);worldcircle(z.pos,Moves[z.move].radius,orange);label(sx(z.pos.x)-24,sy(z.pos.y)-3,"BURN",orange,1);}
 for(auto&o:w.obstacles){worldcircle(o.pos+Vec{100,130},o.radius,{130,131,120,255},true);worldcircle(o.pos,o.radius,{158,159,148,255},true);worldcircle(o.pos,o.radius,black);line(sx(o.pos.x)-17,sy(o.pos.y)-16,sx(o.pos.x)+8,sy(o.pos.y)-23,white);label(sx(o.pos.x)-12,sy(o.pos.y)-3,"ROCK",black,1);}
 for(int i=0;i<2;i++){auto&b=w.bodies[i];SDL_Color team=i?orange:blue;Phase ph=phase(b);
 if(b.move>=0&&(ph==Startup||ph==Active)){const auto&m=Moves[b.move];SDL_Color c=ph==Startup?yellow:orange;Vec end=b.pos+scale(b.locked,m.range);if(m.kind==Melee||m.kind==Lunge){Vec center=b.pos+scale(b.locked,m.range/2);worldcircle(center,m.radius,{c.r,c.g,c.b,70},true);worldcircle(center,m.radius,c);}else if(m.kind==Bolt){line(sx(b.pos.x),sy(b.pos.y),sx(end.x),sy(end.y),c);worldcircle(end,m.radius,c);}else if(m.kind==Field){end=target_point(w,i,b.move);worldcircle(end,m.radius,c);}else{worldcircle(b.pos,b.radius+180,c);}
 }
 if(b.haste)worldcircle(b.pos,b.radius+240,green);if(b.burn)worldcircle(b.pos,b.radius+140,orange);
 worldcircle(b.pos+Vec{80,120},b.radius,{110,110,100,120},true);worldcircle(b.pos,b.radius,team,true);worldcircle(b.pos,b.radius,black);Vec nose=b.pos+scale(b.aim,b.radius+200);line(sx(b.pos.x),sy(b.pos.y),sx(nose.x),sy(nose.y),white);label(sx(b.pos.x)-3,sy(b.pos.y)-3,i?"B":"A",white,1);
 meter(sx(b.pos.x)-24,sy(b.pos.y)-27,48,b.hp,100,green);label(sx(b.pos.x)-int(std::string(phase_name(ph)).size())*3,sy(b.pos.y)+20,phase_name(ph),ph==Startup?orange:dark,1);
 }
 for(auto&p:w.projectiles)if(p.life){worldcircle(p.pos,Moves[p.move].radius+60,orange,true);worldcircle(p.pos,Moves[p.move].radius,yellow);}
 SDL_RenderSetClipRect(r,nullptr);
 if(w.terminal||w.truncated){panel(170,362,400,92);label(190,380,w.truncated?"TIME LIMIT":w.winner<0?"DOUBLE KNOCKOUT":w.winner==0?"CREATURE A WINS":"CREATURE B WINS",blue,2);label(192,416,"RESET TO PLAY / SAVE REPLAY TO INSPECT",black,1);}
 else if(paused){panel(285,164,170,26);label(300,173,"PAUSED / N TO STEP",black,1);}
 label(28,696,"A: BLUE   B: ORANGE   YELLOW: WINDUP   RED: ACTIVE",black,1);label(28,714,"CIRCLES ARE HITBOXES / ROCKS BLOCK SHOTS",dark,1);
 panel(738,108,330,220,true);label(752,121,"CREATURE INSPECTOR",blue,2);
 for(int i=0;i<2;i++){auto&b=w.bodies[i];int y=151+i*80;label(752,y,i?"B / CINDER":"A / SPARK",i?orange:blue,1);label(918,y,manual&&i==0?"HUMAN":"SCRIPTED",dark,1);meter(752,y+17,196,b.hp,100,green);label(958,y+19,"HP "+num(b.hp),black,1);meter(752,y+34,196,b.energy,1000,blue);label(958,y+36,"EN "+num(b.energy/10),black,1);label(752,y+53,b.move<0?"READY":std::string(Moves[b.move].name)+" / "+phase_name(phase(b)),black,1);}
 label(752,311,"BURN "+num(w.bodies[0].burn)+"T  HASTE "+num(w.bodies[0].haste)+"T  WET "+num(w.wetness/10)+"%",dark,1);
 panel(738,338,330,148,true);label(752,350,"MOVE PALETTE / A",blue,1);auto mask=action_mask(w,0);for(int j=0;j<5;j++){int y=370+j*21;label(752,y,(j==4?"SPC ":num(j+1)+"   ")+Moves[j].name,mask[j+1]?black:dark,1);label(942,y,w.bodies[0].cooldown[j]?"CD "+num(w.bodies[0].cooldown[j])+"T":"READY",mask[j+1]?green:dark,1);}
 panel(738,496,330,100);label(752,507,"TRAINER GUIDANCE",blue,1);button(11,750,522,72,"ATTACK");button(12,828,522,72,"RETREAT");button(13,906,522,80,"CONSERVE");button(14,992,522,64,"FREE");button(15,750,558,96,"PRAISE +");button(16,852,558,104,"CORRECT -");button(9,962,558,94,"LOAD REPLAY");
 panel(738,606,330,120,true);label(752,618,"EVENT MONITOR",blue,1);const char*names[]={"", "START", "RELEASE", "HIT", "DODGE", "INTERRUPT", "KO", "GUIDANCE", "POOL FULL", "END"};for(int j=0;j<std::min(6,w.event_count);j++){auto&v=w.events[(w.event_head-1-j+HistoryCount)%HistoryCount];label(752,638+j*13,num(v.tick)+"  "+(v.actor?"B ":"A ")+names[v.kind]+(v.amount?" "+num(v.amount):""),black,1);}
 panel(22,738,1046,22,true);label(30,746,note,black,1);label(950,746,playback?"REPLAY":manual?"MANUAL":"AUTOPLAY",blue,1);
 SDL_RenderPresent(r);rendered++;if(frames_limit&&rendered>=frames_limit){int capture_w,capture_h;SDL_GetRendererOutputSize(r,&capture_w,&capture_h);SDL_Surface*s=SDL_CreateRGBSurfaceWithFormat(0,capture_w,capture_h,32,SDL_PIXELFORMAT_ARGB8888);SDL_RenderReadPixels(r,nullptr,s->format->format,s->pixels,s->pitch);SDL_SaveBMP(s,(captures+"/viewer.bmp").c_str());SDL_FreeSurface(s);run=false;}
 }
 SDL_DestroyRenderer(r);SDL_DestroyWindow(window);SDL_Quit();return 0;
}

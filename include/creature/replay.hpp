#pragma once
#include "sim.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
namespace creature {
struct ReplayFrame {std::array<Action,2> actions{};std::array<int,2> guidance{-1,-1};int feedback=0;uint64_t expected_hash=0;};
struct Replay {World initial{};std::vector<ReplayFrame> frames;};
inline StepResult replay_step(World&w,const ReplayFrame&f){for(int i=0;i<2;i++)if(f.guidance[i]>=0)command(w,i,f.guidance[i]);return step(w,f.actions);}
inline bool save_replay(const Replay&t,const std::string&path){std::ofstream out(path);if(!out)return false;out<<"CREATURE_REPLAY "<<RulesVersion<<"\n";auto s=snapshot(t.initial);for(auto b:s)out<<std::hex<<std::setw(2)<<std::setfill('0')<<int(b);out<<std::dec<<"\n";for(auto&f:t.frames){for(auto&a:f.actions)out<<a.mx<<' '<<a.my<<' '<<a.ax<<' '<<a.ay<<' '<<a.ability<<' ';out<<f.guidance[0]<<' '<<f.guidance[1]<<' '<<f.feedback<<' '<<f.expected_hash<<'\n';}return bool(out);}
// Parse AND verify into temporary storage; malformed files never mutate the caller.
inline bool load_replay(Replay&dst,const std::string&path){std::ifstream in(path,std::ios::binary|std::ios::ate);if(!in||in.tellg()>2*1024*1024)return false;in.seekg(0);std::string magic,hex;uint32_t version;if(!(in>>magic>>version>>hex)||magic!="CREATURE_REPLAY"||version!=RulesVersion||hex.size()!=snapshot(World{}).size()*2)return false;std::vector<uint8_t> s;auto nibble=[](char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;};for(size_t i=0;i<hex.size();i+=2){int a=nibble(hex[i]),b=nibble(hex[i+1]);if(a<0||b<0)return false;s.push_back(uint8_t(a*16+b));}Replay t;if(!restore(t.initial,s.data(),s.size()))return false;World w=t.initial;while(true){in>>std::ws;if(in.eof())break;ReplayFrame f;for(auto&a:f.actions)if(!(in>>a.mx>>a.my>>a.ax>>a.ay>>a.ability))return false;if(!(in>>f.guidance[0]>>f.guidance[1]>>f.feedback>>f.expected_hash)||f.guidance[0]<-1||f.guidance[0]>3||f.guidance[1]<-1||f.guidance[1]>3||f.feedback<-1||f.feedback>1||t.frames.size()>=900||w.terminal||w.truncated)return false;replay_step(w,f);if(hash(w)!=f.expected_hash)return false;t.frames.push_back(f);}dst=std::move(t);return true;}
}

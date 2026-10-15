#pragma once
#include "geometry.hpp"
#include "config.hpp"
#include <cmath>
#include <vector>
namespace hyprv4 {
inline std::vector<Rect> split(Rect area,int count,int gap,bool horizontal=false) {
  area.require_valid(); if(count<0 || count>100000 || gap<0) throw std::invalid_argument("invalid split");
  if(count==0) return {};
  const int size=horizontal?area.width:area.height;
  const int spacing=count==1?0:std::min(gap,size/(count-1));
  const int usable=size-spacing*(count-1),base=usable/count,extra=usable%count;
  std::vector<Rect> result; result.reserve(static_cast<std::size_t>(count));
  int cursor=horizontal?area.x:area.y;
  for(int i=0;i<count;++i){
    const int extent=base+(i<extra?1:0);
    result.push_back(horizontal?Rect{cursor,area.y,extent,area.height}:Rect{area.x,cursor,area.width,extent});
    if(i+1<count) cursor+=extent+spacing;
  }
  return result;
}
inline std::vector<Rect> master_stack(Rect area,int count,const Config& c) {
  c.validate(); area.require_valid();
  if(count<0 || count>100000) throw std::invalid_argument("invalid window count");
  area=inset(area,c.outer_gap);
  const int masters=std::min(count,c.master_count);
  if(count<=masters) return split(area,count,c.gap);
  const int gap=std::min(c.gap,area.width),usable=area.width-gap;
  const int width=static_cast<int>(usable*c.ratio);
  auto result=split({area.x,area.y,width,area.height},masters,c.gap);
  auto stack=split({area.x+width+gap,area.y,usable-width,area.height},count-masters,c.gap);
  result.insert(result.end(),stack.begin(),stack.end()); return result;
}
inline std::vector<Rect> monocle(Rect area,int count,const Config& c) {
  c.validate(); if(count<0 || count>100000) throw std::invalid_argument("invalid window count");
  return std::vector<Rect>(static_cast<std::size_t>(count),inset(area,c.outer_gap));
}
inline std::vector<Rect> grid_layout(Rect area,int count,const Config& c) {
  c.validate(); if(count<0 || count>100000) throw std::invalid_argument("invalid window count");
  if(count==0) return {};
  area=inset(area,c.outer_gap); int columns=1;
  while(columns*columns<count) ++columns;
  const int rows=(count+columns-1)/columns;
  auto bands=split(area,rows,c.gap);std::vector<Rect> result;
  for(const auto& band:bands){
    const int n=std::min(columns,count-static_cast<int>(result.size()));
    auto row=split(band,n,c.gap,true);result.insert(result.end(),row.begin(),row.end());
  }
  return result;
}
inline std::vector<Rect> bottom_stack(Rect area,int count,const Config& c) {
  auto result=master_stack({area.y,area.x,area.height,area.width},count,c);
  for(auto& r:result) r={r.y,r.x,r.height,r.width};
  return result;
}
enum class Layout { MasterStack, Monocle, Grid, BottomStack };
inline std::vector<Rect> arrange(Layout layout,Rect area,int count,const Config& c={}) {
  switch(layout){
  case Layout::MasterStack:return master_stack(area,count,c);
  case Layout::Monocle:return monocle(area,count,c);
  case Layout::Grid:return grid_layout(area,count,c);
  case Layout::BottomStack:return bottom_stack(area,count,c);
  }
  throw std::invalid_argument("unknown layout");
}
inline Rect content_rect(Rect frame,int border) { return inset(frame,border); }
// LAYOUT API
} // namespace hyprv4

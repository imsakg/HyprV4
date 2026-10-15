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
// LAYOUT API
} // namespace hyprv4

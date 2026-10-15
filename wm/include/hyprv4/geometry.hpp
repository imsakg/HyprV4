#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace hyprv4 {
struct Rect {
  int x=0, y=0, width=0, height=0;
  bool valid() const noexcept {
    const auto r=static_cast<std::int64_t>(x)+width;
    const auto b=static_cast<std::int64_t>(y)+height;
    return width>=0 && height>=0 && r<=std::numeric_limits<int>::max() && b<=std::numeric_limits<int>::max();
  }
  void require_valid() const { if(!valid()) throw std::invalid_argument("invalid rectangle"); }
  int right() const { require_valid(); return x+width; }
  int bottom() const { require_valid(); return y+height; }
  bool contains(int px,int py) const { return px>=x && py>=y && px<right() && py<bottom(); }
  friend bool operator==(Rect a,Rect b) { return a.x==b.x && a.y==b.y && a.width==b.width && a.height==b.height; }
};
inline Rect inset(Rect r,int pad) {
  r.require_valid(); if(pad<0) throw std::invalid_argument("negative inset");
  const int px=std::min(pad,r.width/2),py=std::min(pad,r.height/2);
  return {r.x+px,r.y+py,r.width-2*px,r.height-2*py};
}
inline Rect intersection(Rect a,Rect b) {
  a.require_valid(); b.require_valid();
  const int x=std::max(a.x,b.x), y=std::max(a.y,b.y);
  const auto w=std::max<std::int64_t>(0,static_cast<std::int64_t>(std::min(a.right(),b.right()))-x);
  const auto h=std::max<std::int64_t>(0,static_cast<std::int64_t>(std::min(a.bottom(),b.bottom()))-y);
  return {x,y,static_cast<int>(w),static_cast<int>(h)};
}
// GEOMETRY API
} // namespace hyprv4

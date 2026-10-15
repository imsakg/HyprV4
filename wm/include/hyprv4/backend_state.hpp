#pragma once
#include "geometry.hpp"
#include <vector>
#include <map>
#include <set>
namespace hyprv4 {
class StackingOrder {
  std::vector<int> previous_;
public:
  bool update(const std::vector<int>& ids){if(previous_==ids) return false;previous_=ids;return true;}
};
class VisibilityLedger {
  std::set<int> hidden_;std::map<int,unsigned> pending_;
public:
  void hide(int id){hidden_.insert(id);++pending_[id];}
  void show(int id){hidden_.erase(id);}
  bool acknowledge_unmap(int id){const auto i=pending_.find(id);if(i==pending_.end()) return false;if(--i->second==0) pending_.erase(i);return true;}
  void forget(int id){hidden_.erase(id);pending_.erase(id);}
  const std::set<int>& hidden() const {return hidden_;}
};
struct X11Geometry {int x,y;unsigned width,height,border;};
inline X11Geometry x11_geometry(Rect frame,int border,bool fullscreen){
  frame.require_valid();if(border<0) throw std::invalid_argument("negative border");
  const int b=fullscreen?0:std::min(border,std::min(frame.width,frame.height)/2);
  return {frame.x,frame.y,static_cast<unsigned>(std::max(1,frame.width-2*b)),static_cast<unsigned>(std::max(1,frame.height-2*b)),static_cast<unsigned>(b)};
}
// BACKEND API
}

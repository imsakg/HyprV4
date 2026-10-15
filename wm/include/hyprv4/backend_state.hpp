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
// BACKEND API
}

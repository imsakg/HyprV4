#pragma once
#include "geometry.hpp"
#include <vector>
namespace hyprv4 {
class StackingOrder {
  std::vector<int> previous_;
public:
  bool update(const std::vector<int>& ids){if(previous_==ids) return false;previous_=ids;return true;}
};
// BACKEND API
}

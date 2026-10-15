#include "check.hpp"
#include "hyprv4/geometry.hpp"
using namespace hyprv4;
int main() {
  Rect r{10,20,100,80}; CHECK(r.valid()); CHECK(r.right()==110); CHECK(r.bottom()==100);
  CHECK(r.contains(10,20)); CHECK(!r.contains(110,100)); CHECK(r==Rect{10,20,100,80});
  CHECK(!Rect{0,0,-1,10}.valid());
  // NEXT TEST
}

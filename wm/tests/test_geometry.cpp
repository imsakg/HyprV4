#include "check.hpp"
#include "hyprv4/geometry.hpp"
using namespace hyprv4;
int main() {
  Rect r{10,20,100,80}; CHECK(r.valid()); CHECK(r.right()==110); CHECK(r.bottom()==100);
  CHECK(r.contains(10,20)); CHECK(!r.contains(110,100)); CHECK(r==Rect{10,20,100,80});
  CHECK(!Rect{0,0,-1,10}.valid());
    CHECK(inset(Rect{0,0,101,51},4)==Rect{4,4,93,43});
  CHECK(inset(Rect{0,0,3,1},100)==Rect{1,0,1,1});
  bool bad_inset=false;try{inset(Rect{},-1);}catch(const std::invalid_argument&){bad_inset=true;} CHECK(bad_inset);
  CHECK(intersection(Rect{0,0,100,100},Rect{50,25,100,100})==Rect{50,25,50,75});
  CHECK(intersection(Rect{0,0,10,10},Rect{20,20,4,4})==Rect{20,20,0,0});
  CHECK(translate(Rect{3,4,10,10},-8,6)==Rect{-5,10,10,10});
  bool overflow=false;try{translate(Rect{2147483640,0,1,1},20,0);}catch(const std::overflow_error&){overflow=true;} CHECK(overflow);
  CHECK(clamp_to(Rect{-5,90,40,40},Rect{0,0,100,100})==Rect{0,60,40,40});
  CHECK(clamp_to(Rect{5,5,300,200},Rect{10,20,100,80})==Rect{10,20,100,80});
  CHECK(reserve(Rect{10,20,100,80},Insets{5,8,3,4})==Rect{15,23,87,73});
  CHECK(reserve(Rect{0,0,10,10},Insets{20,20,20,20})==Rect{10,10,0,0});
// NEXT TEST
}

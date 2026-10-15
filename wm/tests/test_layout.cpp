#include "check.hpp"
#include "hyprv4/layout.hpp"
using namespace hyprv4;
int main(){
  auto v=split(Rect{0,0,101,101},3,2); CHECK(v.size()==3); CHECK(v[0]==Rect{0,0,101,33});
  CHECK(v[1].y==35); CHECK(v.back().bottom()==101); CHECK(split(Rect{},0,2).empty());
  // NEXT TEST
}

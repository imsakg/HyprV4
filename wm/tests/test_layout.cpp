#include "check.hpp"
#include "hyprv4/layout.hpp"
using namespace hyprv4;
int main(){
  auto v=split(Rect{0,0,101,101},3,2); CHECK(v.size()==3); CHECK(v[0]==Rect{0,0,101,33});
  CHECK(v[1].y==35); CHECK(v.back().bottom()==101); CHECK(split(Rect{},0,2).empty());
    Config c;c.gap=10;c.outer_gap=0;c.ratio=0.6;
  auto m=master_stack(Rect{0,0,1010,600},3,c); CHECK(m[0]==Rect{0,0,600,600});
  CHECK(m[1]==Rect{610,0,400,295}); CHECK(m[2].bottom()==600);
  c.master_count=8;CHECK(master_stack(Rect{0,0,100,100},2,c)[0].width==100);
// NEXT TEST
}

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
  c.master_count=1;c.outer_gap=7;c.gap=0;
  CHECK(master_stack(Rect{0,0,100,80},1,c)[0]==Rect{7,7,86,66});
  auto mono=monocle(Rect{0,0,100,80},3,c);CHECK(mono.size()==3);
  CHECK(mono[0]==Rect{7,7,86,66});CHECK(mono[2]==mono[0]);
  c.outer_gap=0;c.gap=4;
  auto grid=grid_layout(Rect{0,0,300,200},5,c);CHECK(grid.size()==5);
  CHECK(grid[0].y==0); CHECK(grid[3].y==102); CHECK(grid.back().right()==300);
  CHECK(grid[3].width==148);
  c.gap=0;c.ratio=0.5;
  auto bs=bottom_stack(Rect{10,20,400,200},3,c);CHECK(bs[0]==Rect{10,20,400,100});
  CHECK(bs[1]==Rect{10,120,200,100});CHECK(bs[2].right()==410);
  CHECK(arrange(Layout::Grid,Rect{0,0,80,80},3,c).size()==3);
  CHECK(arrange(Layout::Monocle,Rect{0,0,80,80},2,c)[1]==Rect{0,0,80,80});
  bool invalid_layout=false;try{arrange(static_cast<Layout>(99),Rect{},0,c);}catch(const std::invalid_argument&){invalid_layout=true;}CHECK(invalid_layout);
// NEXT TEST
}

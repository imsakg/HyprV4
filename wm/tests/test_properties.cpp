#include "check.hpp"
#include "hyprv4/manager.hpp"
#include <random>
using namespace hyprv4;
int main(){
  for(int width:{0,1,2,31,101,1920}) for(int height:{0,1,7,99,1080}) for(int count=0;count<35;++count){
    Config c;c.outer_gap=9;c.gap=11;
    for(auto layout:{Layout::MasterStack,Layout::Monocle,Layout::Grid,Layout::BottomStack}){
      const Rect area{-100,-100,width,height};const auto result=arrange(layout,area,count,c);CHECK(result.size()==static_cast<std::size_t>(count));
      for(auto r:result){CHECK(r.valid());CHECK(r.x>=area.x);CHECK(r.y>=area.y);CHECK(r.right()<=area.right());CHECK(r.bottom()<=area.bottom());}
    }
  }
  std::mt19937 random(7241);Manager wm;wm.add_workspace(2);wm.add_workspace(3);int next=1;
  for(int i=0;i<5000;++i){const int operation=static_cast<int>(random()%12),id=1+static_cast<int>(random()%static_cast<unsigned>(next));
    if(operation==0) wm.manage(next++);
    else if(operation==1) wm.switch_workspace(1+static_cast<int>(random()%3));
    else if(wm.find(id)){
      switch(operation){
      case 2:wm.unmanage(id);break;case 3:wm.toggle_floating(id);break;
      case 4:wm.set_fullscreen(id,random()%2!=0);break;case 5:wm.minimize(id,random()%2!=0);break;
      case 6:wm.move_to_workspace(id,1+static_cast<int>(random()%3));break;
      case 7:wm.stash(id);break;case 8:wm.restore(id);break;case 9:wm.focus(id);break;
      case 10:wm.cycle_focus(static_cast<int>(random()%7)-3);break;case 11:wm.promote_master();break;
      }
    }
    CHECK(wm.valid_state());
  }
}

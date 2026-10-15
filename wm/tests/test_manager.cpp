#include "check.hpp"
#include "hyprv4/manager.hpp"
using namespace hyprv4;
int main(){
  Manager wm;wm.manage(10,"Terminal");CHECK(wm.size()==1);CHECK(wm.find(10)->title=="Terminal");
  CHECK(!wm.find(99));bool duplicate=false;try{wm.manage(10);}catch(const std::invalid_argument&){duplicate=true;}CHECK(duplicate);
    wm.manage(11);CHECK(wm.unmanage(11));CHECK(!wm.unmanage(11));CHECK(wm.size()==1);CHECK(!wm.find(11));
  CHECK(wm.focused()==10);CHECK(wm.focus(10));CHECK(!wm.focus(999));CHECK(wm.focused()==10);
  wm.manage(12);wm.manage(13);CHECK(wm.cycle_focus(1)==10);CHECK(wm.cycle_focus(-1)==13);CHECK(wm.cycle_focus(5)==12);
  auto placements=wm.placements();CHECK(placements.size()==3);CHECK(placements[0].id==10);
  wm.set_layout(Layout::Monocle);CHECK(wm.placements()[0].frame==wm.placements()[2].frame);
  wm.add_workspace(2);wm.switch_workspace(2);CHECK(wm.placements().empty());CHECK(!wm.focus(10));
  wm.manage(20);CHECK(wm.focused()==20);wm.switch_workspace(1);CHECK(wm.focused()==12);
  wm.move_to_workspace(13,2);CHECK(wm.find(13)->workspace==2);CHECK(wm.placements().size()==2);
  wm.switch_workspace(2);CHECK(wm.focused()==20);CHECK(wm.placements().size()==2);wm.switch_workspace(1);
  wm.toggle_floating(12);CHECK(wm.find(12)->floating);CHECK(wm.placements().size()==2);
  CHECK(wm.placements()[1].frame==Rect{100,100,640,480});
  wm.configure(12,Rect{-100,1000,800,200});CHECK(wm.find(12)->frame==Rect{0,880,800,200});
  bool tiled_configure=false;try{wm.configure(10,Rect{0,0,1,1});}catch(const std::invalid_argument&){tiled_configure=true;}CHECK(tiled_configure);
  wm.set_fullscreen(10,true);CHECK(wm.placements().size()==1);CHECK(wm.placements()[0].frame==Rect{0,0,1920,1080});
  wm.set_fullscreen(12,true);CHECK(!wm.find(10)->fullscreen);CHECK(wm.placements()[0].id==12);wm.set_fullscreen(12,false);
  wm.minimize(12,true);CHECK(wm.find(12)->minimized);CHECK(wm.placements().size()==1);CHECK(wm.focused()==10);CHECK(!wm.focus(12));CHECK(wm.cycle_focus(1)==10);
  wm.minimize(10,true);CHECK(!wm.focused());CHECK(wm.placements().empty());wm.minimize(10,false);wm.minimize(12,false);
  wm.set_urgent(12,true);CHECK(wm.find(12)->urgent);CHECK(wm.focus(12));CHECK(!wm.find(12)->urgent);
  wm.set_urgent(12,true);CHECK(!wm.find(12)->urgent);wm.focus(10);
  wm.set_title(10,"Terminal — build output");CHECK(wm.find(10)->title=="Terminal — build output");
  wm.toggle_floating(12);wm.focus(10);CHECK(wm.swap_focused(1));CHECK(wm.placements()[0].id==12);CHECK(wm.focused()==10);
  CHECK(wm.promote_master());CHECK(wm.placements()[0].id==10);CHECK(wm.focused()==10);
  wm.toggle_floating(12);wm.configure(12,Rect{100,100,400,300});wm.move_floating(12,20,-30);CHECK(wm.find(12)->frame==Rect{120,70,400,300});
  wm.resize_floating(12,20,30);CHECK(wm.find(12)->frame==Rect{120,70,420,330});
  bool negative_resize=false;try{wm.resize_floating(12,-500,0);}catch(const std::invalid_argument&){negative_resize=true;}CHECK(negative_resize);CHECK(wm.find(12)->frame.width==420);
// NEXT TEST
}

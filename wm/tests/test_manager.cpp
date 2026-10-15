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
// NEXT TEST
}

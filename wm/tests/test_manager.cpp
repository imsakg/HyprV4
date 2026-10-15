#include "check.hpp"
#include "hyprv4/manager.hpp"
using namespace hyprv4;
int main(){
  Manager wm;wm.manage(10,"Terminal");CHECK(wm.size()==1);CHECK(wm.find(10)->title=="Terminal");
  CHECK(!wm.find(99));bool duplicate=false;try{wm.manage(10);}catch(const std::invalid_argument&){duplicate=true;}CHECK(duplicate);
    wm.manage(11);CHECK(wm.unmanage(11));CHECK(!wm.unmanage(11));CHECK(wm.size()==1);CHECK(!wm.find(11));
  CHECK(wm.focused()==10);CHECK(wm.focus(10));CHECK(!wm.focus(999));CHECK(wm.focused()==10);
// NEXT TEST
}

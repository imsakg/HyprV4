#include "check.hpp"
#include "hyprv4/manager.hpp"
using namespace hyprv4;
int main(){Manager wm;wm.manage(1);wm.toggle_floating(1);wm.manage(2);auto c=wm.config();c.focus_follows_mouse=true;wm.set_config(c);CHECK(wm.placements().back().id==1);CHECK(wm.pointer_enter(200,200)==1);}

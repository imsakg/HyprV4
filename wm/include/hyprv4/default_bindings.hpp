#pragma once
#include "keybindings.hpp"
namespace hyprv4 {
inline KeyBindings default_bindings(){
  KeyBindings keys;keys.bind("Super+j","cycle",{"1"});keys.bind("Super+k","cycle",{"-1"});
  keys.bind("Super+space","floating-focused");keys.bind("Super+f","fullscreen-focused");keys.bind("Super+q","close-focused");keys.bind("Super+Shift+Escape","quit");
  for(int i=1;i<=9;++i){const auto n=std::to_string(i);keys.bind("Super+"+n,"workspace",{n});keys.bind("Super+Shift+"+n,"move-focused",{n});}
  return keys;
}
}

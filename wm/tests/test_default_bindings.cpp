#include "check.hpp"
#include "hyprv4/default_bindings.hpp"
using namespace hyprv4;
int main(){auto keys=default_bindings();CHECK(keys.lookup("Super+j")->arguments==std::vector<std::string>{"1"});CHECK(keys.lookup("Super+Shift+3")->action=="move-focused");CHECK(keys.lookup("Super+Shift+Escape")->action=="quit");}

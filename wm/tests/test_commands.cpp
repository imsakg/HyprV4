#include "check.hpp"
#include "hyprv4/commands.hpp"
using namespace hyprv4;
int main(){
  CHECK(tokenize("manage 1 'Hello world' # note")==std::vector<std::string>({"manage","1","Hello world"}));
  CHECK(tokenize("bind Super+Return terminal \"kitty\"").size()==4);
  bool quote=false;try{tokenize("'unterminated");}catch(const std::invalid_argument&){quote=true;}CHECK(quote);
  // NEXT TEST
}

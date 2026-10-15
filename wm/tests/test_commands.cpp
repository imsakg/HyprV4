#include "check.hpp"
#include "hyprv4/commands.hpp"
using namespace hyprv4;
int main(){
  CHECK(tokenize("manage 1 'Hello world' # note")==std::vector<std::string>({"manage","1","Hello world"}));
  CHECK(tokenize("bind Super+Return terminal \"kitty\"").size()==4);
  bool quote=false;try{tokenize("'unterminated");}catch(const std::invalid_argument&){quote=true;}CHECK(quote);
    CommandRunner runner;CHECK(runner.execute("manage 1 terminal")=="ok");runner.execute("manage 2 editor");runner.execute("focus 1");CHECK(runner.state.focused()==1);
  bool arity=false;try{runner.execute("focus 1 extra");}catch(const std::invalid_argument&){arity=true;}CHECK(arity);
  runner.execute("cycle 1");CHECK(runner.state.focused()==2);
// NEXT TEST
}

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
  runner.execute("workspace-add 2");runner.execute("move 2 2");runner.execute("workspace 2");runner.execute("floating 2");runner.execute("configure 2 10 20 300 200");CHECK(runner.state.find(2)->frame==Rect{10,20,300,200});
  runner.execute("layout grid");CHECK(runner.state.layout()==Layout::Grid);runner.execute("fullscreen 2 1");CHECK(runner.state.find(2)->fullscreen);
  runner.reload("gap=3\nratio=0.4\n");CHECK(runner.state.config().gap==3);
  bool reload=false;try{runner.reload("gap=99\nratio=nan");}catch(const std::invalid_argument&){reload=true;}CHECK(reload);CHECK(runner.state.config().gap==3);
  runner.execute("bind Super+j cycle 1");CHECK(runner.execute("key Win+J")=="ok");CHECK(runner.execute("key Alt+x")=="unbound");
  runner.execute("bind x key x");bool recursion=false;try{runner.execute("key x");}catch(const std::invalid_argument&){recursion=true;}CHECK(recursion);CHECK(runner.execute("key Alt+x")=="unbound");
// NEXT TEST
}

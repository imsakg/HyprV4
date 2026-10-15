#pragma once
#include "manager.hpp"
#include "keybindings.hpp"
#include <charconv>
#include <sstream>
namespace hyprv4 {
inline std::vector<std::string> tokenize(const std::string& line){
  std::vector<std::string> result;std::string token;char quote=0;bool escaped=false,started=false;
  for(char c:line){
    if(escaped){token+=c;escaped=false;started=true;continue;}
    if(c=='\\' && quote!='\''){escaped=true;started=true;continue;}
    if(quote){if(c==quote) quote=0;else token+=c;started=true;continue;}
    if(c=='\'' || c=='"'){quote=c;started=true;continue;}
    if(c=='#') break;
    if(c==' ' || c=='\t' || c=='\r'){if(started){result.push_back(token);token.clear();started=false;}continue;}
    token+=c;started=true;
  }
  if(quote || escaped) throw std::invalid_argument("unfinished quote or escape");
  if(started) result.push_back(token);return result;
}
class CommandRunner {
public:
  Manager state;KeyBindings keys;
  std::string execute(const std::string& line){return execute_tokens(tokenize(line));}
  std::string execute_tokens(const std::vector<std::string>& args){
    if(args.empty()) return {};
    const auto& name=args[0];
    if(name=="manage"){arity(args,2,4);state.manage(number(args[1]),args.size()>2?args[2]:"",args.size()>3?args[3]:"");}
    else if(name=="unmanage"){arity(args,2,2);if(!state.unmanage(number(args[1]))) throw std::invalid_argument("unknown client");}
    else if(name=="focus"){arity(args,2,2);if(!state.focus(number(args[1]))) throw std::invalid_argument("client not focusable");}
    else if(name=="cycle"){arity(args,2,2);state.cycle_focus(number(args[1]));}
    // COMMAND ACTIONS
    else throw std::invalid_argument("unknown command: "+name);
    return "ok";
  }
private:
  static void arity(const std::vector<std::string>& args,std::size_t min,std::size_t max){if(args.size()<min || args.size()>max) throw std::invalid_argument("wrong argument count for "+args[0]);}
  static int number(const std::string& value){int result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) throw std::invalid_argument("expected integer");return result;}
  // COMMAND PRIVATE
};
// COMMAND API
} // namespace hyprv4

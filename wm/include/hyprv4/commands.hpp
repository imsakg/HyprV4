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
inline std::string json_string(const std::string& input){
  constexpr char hex[]="0123456789abcdef";std::string out="\"";
  for(unsigned char c:input){switch(c){case '"':out+="\\\"";break;case '\\':out+="\\\\";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:if(c<32){out+="\\u00";out+=hex[c>>4];out+=hex[c&15];}else out+=static_cast<char>(c);}}
  return out+"\"";
}
class CommandRunner {
public:
  Manager state;KeyBindings keys;
  void reload(const std::string& text){state.set_config(Config::parse(text));}
  std::string snapshot() const {
    std::ostringstream out;out<<"{\"workspace\":"<<state.current_workspace()<<",\"focused\":";
    if(state.focused()) out<<*state.focused();else out<<"null";
    out<<",\"windows\":[";bool first=true;
    for(const auto& p:state.placements()){if(!first) out<<',';first=false;out<<"{\"id\":"<<p.id<<",\"title\":"<<json_string(state.find(p.id)->title)<<",\"frame\":["<<p.frame.x<<','<<p.frame.y<<','<<p.frame.width<<','<<p.frame.height<<"]}";}
    return out.str()+"]}";
  }
  std::string execute(const std::string& line){return execute_tokens(tokenize(line));}
  std::string execute_tokens(const std::vector<std::string>& args){
    if(args.empty()) return {};
    const auto& name=args[0];
    if(name=="manage"){arity(args,2,4);state.manage(number(args[1]),args.size()>2?args[2]:"",args.size()>3?args[3]:"");}
    else if(name=="unmanage"){arity(args,2,2);if(!state.unmanage(number(args[1]))) throw std::invalid_argument("unknown client");}
    else if(name=="focus"){arity(args,2,2);if(!state.focus(number(args[1]))) throw std::invalid_argument("client not focusable");}
    else if(name=="cycle"){arity(args,2,2);state.cycle_focus(number(args[1]));}
    else if(name=="workspace-add"){arity(args,2,2);state.add_workspace(number(args[1]));}
    else if(name=="workspace"){arity(args,2,2);state.switch_workspace(number(args[1]));}
    else if(name=="move"){arity(args,3,3);state.move_to_workspace(number(args[1]),number(args[2]));}
    else if(name=="floating"){arity(args,2,2);state.toggle_floating(number(args[1]));}
    else if(name=="fullscreen"){arity(args,3,3);state.set_fullscreen(number(args[1]),boolean(args[2]));}
    else if(name=="minimize"){arity(args,3,3);state.minimize(number(args[1]),boolean(args[2]));}
    else if(name=="close"){arity(args,2,2);state.request_close(number(args[1]));}
    else if(name=="configure"){arity(args,6,6);state.configure(number(args[1]),{number(args[2]),number(args[3]),number(args[4]),number(args[5])});}
    else if(name=="layout"){arity(args,2,2);if(args[1]=="master") state.set_layout(Layout::MasterStack);else if(args[1]=="monocle") state.set_layout(Layout::Monocle);else if(args[1]=="grid") state.set_layout(Layout::Grid);else if(args[1]=="bottom") state.set_layout(Layout::BottomStack);else throw std::invalid_argument("unknown layout");}
    else if(name=="config"){arity(args,2,2);reload(args[1]);}
    else if(name=="bind"){arity(args,3,100);keys.bind(args[1],args[2],std::vector<std::string>(args.begin()+3,args.end()));}
    else if(name=="unbind"){arity(args,2,2);keys.unbind(args[1]);}
    else if(name=="key"){
      arity(args,2,2);if(dispatch_depth_>=16) throw std::invalid_argument("binding recursion limit");
      ++dispatch_depth_;std::string result="unbound";
      try{keys.dispatch(args[1],[&](const Binding& binding){std::vector<std::string> action{binding.action};action.insert(action.end(),binding.arguments.begin(),binding.arguments.end());result=execute_tokens(action);});}
      catch(...){--dispatch_depth_;throw;}
      --dispatch_depth_;return result;
    }
    else if(name=="state"){arity(args,1,1);return snapshot();}
    // COMMAND ACTIONS
    else throw std::invalid_argument("unknown command: "+name);
    return "ok";
  }
private:
  static void arity(const std::vector<std::string>& args,std::size_t min,std::size_t max){if(args.size()<min || args.size()>max) throw std::invalid_argument("wrong argument count for "+args[0]);}
  static int number(const std::string& value){int result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);if(parsed.ec!=std::errc{} || parsed.ptr!=value.data()+value.size()) throw std::invalid_argument("expected integer");return result;}
  static bool boolean(const std::string& value){if(value=="1" || value=="true") return true;if(value=="0" || value=="false") return false;throw std::invalid_argument("expected boolean");}
  int dispatch_depth_=0;
  // COMMAND PRIVATE
};
// COMMAND API
} // namespace hyprv4

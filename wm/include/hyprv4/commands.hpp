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
// COMMAND API
} // namespace hyprv4

#pragma once
#include "layout.hpp"
#include <map>
#include <optional>
#include <string>
#include <utility>
namespace hyprv4 {
struct Window {
  int id=0,workspace=1;
  std::string title,app_id;
  Rect frame{100,100,640,480};
  bool floating=false,fullscreen=false,minimized=false,urgent=false,scratch=false,close_requested=false;
};
struct Workspace {
  int id=1;std::vector<int> windows;std::optional<int> focused;
  Layout layout=Layout::MasterStack;Config config;
};
struct Placement { int id;Rect frame; };
class Manager {
public:
  Manager(){workspaces_.emplace(1,Workspace{});}
  void manage(int id,std::string title={},std::string app_id={}) {
    if(id<=0 || clients_.count(id)) throw std::invalid_argument("invalid or duplicate client id");
    if(ws().windows.size()>=100000) throw std::invalid_argument("workspace client limit");
    Window w;w.id=id;w.workspace=current_;w.title=std::move(title);w.app_id=std::move(app_id);
    clients_.emplace(id,std::move(w));ws().windows.push_back(id);ws().focused=id;
  }
  const Window* find(int id) const { auto i=clients_.find(id);return i==clients_.end()?nullptr:&i->second; }
  std::size_t size() const {return clients_.size();}
  int current_workspace() const {return current_;}
  bool unmanage(int id) {
    auto found=clients_.find(id);if(found==clients_.end()) return false;
    auto& space=workspaces_.at(found->second.workspace);
    space.windows.erase(std::remove(space.windows.begin(),space.windows.end(),id),space.windows.end());
    if(space.focused==id) space.focused=space.windows.empty()?std::nullopt:std::optional<int>(space.windows.back());
    clients_.erase(found);return true;
  }
  std::optional<int> focused() const {return ws().focused;}
  bool focus(int id) {
    auto found=clients_.find(id);if(found==clients_.end() || found->second.workspace!=current_) return false;
    ws().focused=id;return true;
  }
  std::optional<int> cycle_focus(int direction) {
    const auto& ids=ws().windows;if(ids.empty()) return std::nullopt;
    const auto it=std::find(ids.begin(),ids.end(),ws().focused.value_or(-1));
    const auto n=static_cast<std::int64_t>(ids.size());
    const auto index=it==ids.end()?0:it-ids.begin();
    const auto next=(index+static_cast<std::int64_t>(direction)%n+n)%n;
    ws().focused=ids[static_cast<std::size_t>(next)];return ws().focused;
  }
  void set_layout(Layout layout) {(void)arrange(layout,output_,0,ws().config);ws().layout=layout;}
  Layout layout() const {return ws().layout;}
  std::vector<Placement> placements() const {
    int tiled=0;for(int id:ws().windows) if(!clients_.at(id).floating) ++tiled;
    auto rects=arrange(ws().layout,output_,tiled,ws().config);
    std::vector<Placement> result;std::size_t index=0;
    for(int id:ws().windows) {const auto& w=clients_.at(id);result.push_back({id,w.floating?w.frame:rects[index++]});}
    return result;
  }
  void add_workspace(int id) {
    if(id<=0 || workspaces_.count(id)) throw std::invalid_argument("invalid or duplicate workspace");
    Workspace space;space.id=id;space.config=ws().config;workspaces_.emplace(id,std::move(space));
  }
  void switch_workspace(int id) {
    if(!workspaces_.count(id)) throw std::invalid_argument("unknown workspace");current_=id;
  }
  void move_to_workspace(int id,int target) {
    auto& window=get(id);if(!workspaces_.count(target)) throw std::invalid_argument("unknown workspace");
    if(window.workspace==target) return;
    auto& source=workspaces_.at(window.workspace);auto& destination=workspaces_.at(target);
    if(destination.windows.size()>=100000) throw std::invalid_argument("workspace client limit");
    source.windows.erase(std::remove(source.windows.begin(),source.windows.end(),id),source.windows.end());
    if(source.focused==id) source.focused=source.windows.empty()?std::nullopt:std::optional<int>(source.windows.back());
    destination.windows.push_back(id);if(!destination.focused) destination.focused=id;window.workspace=target;
  }
  void toggle_floating(int id){auto& w=get(id);w.floating=!w.floating;w.frame=clamp_to(w.frame,output_);}
  void configure(int id,Rect frame){auto& w=get(id);if(!w.floating) throw std::invalid_argument("cannot configure tiled client");w.frame=clamp_to(frame,output_);}
  // MANAGER API
private:
  std::map<int,Window> clients_;std::map<int,Workspace> workspaces_;int current_=1;
  Rect output_{0,0,1920,1080};Insets margins_;
  Workspace& ws(){return workspaces_.at(current_);}
  const Workspace& ws() const {return workspaces_.at(current_);}
  Window& get(int id){auto it=clients_.find(id);if(it==clients_.end()) throw std::invalid_argument("unknown client");return it->second;}
  // MANAGER PRIVATE
};
} // namespace hyprv4

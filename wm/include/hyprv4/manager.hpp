#pragma once
#include "layout.hpp"
#include <map>
#include <optional>
#include <set>
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
struct Rule {std::string app_id;std::optional<bool> floating;std::optional<int> workspace;};
struct Placement { int id;Rect frame; };
class Manager {
public:
  Manager(){workspaces_.emplace(1,Workspace{});}
  void manage(int id,std::string title={},std::string app_id={}) {
    if(id<=0 || clients_.count(id)) throw std::invalid_argument("invalid or duplicate client id");
    if(ws().windows.size()>=100000) throw std::invalid_argument("workspace client limit");
    Window w;w.id=id;w.workspace=current_;w.title=std::move(title);w.app_id=std::move(app_id);
    clients_.emplace(id,std::move(w));ws().windows.push_back(id);ws().focused=id;apply_rules(id);repair_focus(ws());
  }
  const Window* find(int id) const { auto i=clients_.find(id);return i==clients_.end()?nullptr:&i->second; }
  std::size_t size() const {return clients_.size();}
  int current_workspace() const {return current_;}
  bool unmanage(int id) {
    auto found=clients_.find(id);if(found==clients_.end()) return false;
    auto& space=workspaces_.at(found->second.workspace);
    space.windows.erase(std::remove(space.windows.begin(),space.windows.end(),id),space.windows.end());
    repair_focus(space);
    clients_.erase(found);return true;
  }
  std::optional<int> focused() const {return ws().focused;}
  bool focus(int id) {
    auto found=clients_.find(id);if(found==clients_.end() || found->second.workspace!=current_ || found->second.minimized || found->second.scratch) return false;
    const auto candidates=focus_candidates(ws());if(std::find(candidates.begin(),candidates.end(),id)==candidates.end()) return false;
    ws().focused=id;found->second.urgent=false;return true;
  }
  std::optional<int> cycle_focus(int direction) {
    const auto ids=focus_candidates(ws());if(ids.empty()){ws().focused.reset();return std::nullopt;}
    const auto it=std::find(ids.begin(),ids.end(),ws().focused.value_or(-1));
    const auto n=static_cast<std::int64_t>(ids.size());
    const auto index=it==ids.end()?0:it-ids.begin();
    const auto next=(index+static_cast<std::int64_t>(direction)%n+n)%n;
    focus(ids[static_cast<std::size_t>(next)]);return ws().focused;
  }
  void set_layout(Layout layout) {(void)arrange(layout,output_,0,ws().config);ws().layout=layout;}
  Layout layout() const {return ws().layout;}
  std::vector<Placement> placements() const {
    for(int id:ws().windows) if(!clients_.at(id).minimized && !clients_.at(id).scratch && clients_.at(id).fullscreen) return {{id,output_}};
    int tiled=0;for(int id:ws().windows) if(!clients_.at(id).minimized && !clients_.at(id).scratch && !clients_.at(id).floating) ++tiled;
    auto rects=arrange(ws().layout,work_area(),tiled,ws().config);
    std::vector<Placement> result;std::size_t index=0;
    for(int id:ws().windows) {const auto& w=clients_.at(id);if(w.minimized || w.scratch) continue;result.push_back({id,w.floating?w.frame:rects[index++]});}
    std::stable_partition(result.begin(),result.end(),[&](const Placement& p){return !clients_.at(p.id).floating;});
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
    repair_focus(source);
    destination.windows.push_back(id);window.workspace=target;
    if(window.fullscreen){for(int peer:destination.windows) if(peer!=id) get(peer).fullscreen=false;destination.focused=id;}
    repair_focus(destination);
  }
  void toggle_floating(int id){auto& w=get(id);w.floating=!w.floating;w.frame=clamp_to(w.frame,work_area());}
  void configure(int id,Rect frame){auto& w=get(id);if(!w.floating) throw std::invalid_argument("cannot configure tiled client");w.frame=clamp_to(frame,work_area());}
  void set_fullscreen(int id,bool enabled){auto& w=get(id);if(enabled){w.minimized=false;w.scratch=false;for(int peer:workspaces_.at(w.workspace).windows) get(peer).fullscreen=false;workspaces_.at(w.workspace).focused=id;}w.fullscreen=enabled;}
  void minimize(int id,bool enabled){auto& w=get(id);w.minimized=enabled;if(enabled) w.fullscreen=false;repair_focus(workspaces_.at(w.workspace));}
  void set_urgent(int id,bool urgent){auto& w=get(id);w.urgent=urgent && !(w.workspace==current_ && focused()==id);}
  void set_title(int id,std::string title){get(id).title=std::move(title);}
  bool swap_focused(int direction){
    if(!focused()) return false;
    std::vector<int> ids;for(int id:ws().windows) if(!get(id).floating && !get(id).minimized && !get(id).scratch) ids.push_back(id);
    const auto it=std::find(ids.begin(),ids.end(),*focused());if(it==ids.end() || ids.size()<2) return false;
    const auto n=static_cast<std::int64_t>(ids.size());const auto i=it-ids.begin();const auto j=(i+static_cast<std::int64_t>(direction)%n+n)%n;
    std::iter_swap(std::find(ws().windows.begin(),ws().windows.end(),ids[static_cast<std::size_t>(i)]),std::find(ws().windows.begin(),ws().windows.end(),ids[static_cast<std::size_t>(j)]));return true;
  }
  bool promote_master(){
    if(!focused() || get(*focused()).floating) return false;
    auto& ids=ws().windows;const auto it=std::find(ids.begin(),ids.end(),*focused());
    const int id=*it;ids.erase(it);ids.insert(ids.begin(),id);return true;
  }
  void move_floating(int id,int dx,int dy){configure(id,translate(get(id).frame,dx,dy));}
  void resize_floating(int id,int dw,int dh){auto frame=get(id).frame;frame.width=checked_int(static_cast<std::int64_t>(frame.width)+dw);frame.height=checked_int(static_cast<std::int64_t>(frame.height)+dh);configure(id,frame);}
  const Config& config() const {return ws().config;}
  void set_config(const Config& value){value.validate();ws().config=value;}
  Rect output() const {return output_;}
  void set_output(Rect area){area.require_valid();if(area.width==0 || area.height==0) throw std::invalid_argument("empty output");output_=area;for(auto& item:clients_) if(item.second.floating) item.second.frame=clamp_to(item.second.frame,work_area());}
  Rect work_area() const {return reserve(output_,margins_);}
  void set_reserved(Insets margins){(void)reserve(output_,margins);margins_=margins;for(auto& item:clients_) if(item.second.floating) item.second.frame=clamp_to(item.second.frame,work_area());}
  void add_rule(Rule rule){if(rule.app_id.empty() || (rule.workspace && !workspaces_.count(*rule.workspace))) throw std::invalid_argument("invalid rule");rules_.push_back(std::move(rule));}
  std::optional<int> pointer_enter(int x,int y){
    if(!config().focus_follows_mouse) return std::nullopt;
    const auto visible=placements();for(auto it=visible.rbegin();it!=visible.rend();++it) if(it->frame.contains(x,y)){focus(it->id);return it->id;}
    return std::nullopt;
  }
  void stash(int id){auto& w=get(id);w.scratch=true;w.fullscreen=false;repair_focus(workspaces_.at(w.workspace));}
  void restore(int id){get(id);move_to_workspace(id,current_);auto& w=get(id);w.scratch=false;w.minimized=false;w.floating=true;w.frame=clamp_to(w.frame,work_area());focus(id);}
  void remove_workspace(int id,int target){
    if(id==target || !workspaces_.count(id) || !workspaces_.count(target)) throw std::invalid_argument("invalid workspace migration");
    const auto ids=workspaces_.at(id).windows;
    if(ids.size()+workspaces_.at(target).windows.size()>100000) throw std::invalid_argument("workspace client limit");
    for(int client:ids) move_to_workspace(client,target);
    if(current_==id) current_=target;
    for(auto& rule:rules_) if(rule.workspace==id) rule.workspace=target;
    workspaces_.erase(id);
  }
  void request_close(int id){get(id).close_requested=true;}
  std::vector<int> close_requests() const {std::vector<int> ids;for(const auto& item:clients_) if(item.second.close_requested) ids.push_back(item.first);return ids;}
  std::vector<int> workspace_ids() const {std::vector<int> ids;for(const auto& item:workspaces_) ids.push_back(item.first);return ids;}
  bool valid_state() const {
    if(!workspaces_.count(current_) || !output_.valid()) return false;
    std::set<int> seen;
    for(const auto& item:workspaces_){const auto& space=item.second;int full=0;
      for(int id:space.windows){const auto* w=find(id);if(!w || w->workspace!=item.first || w->id!=id || !seen.insert(id).second || !w->frame.valid()) return false;if(w->fullscreen) ++full;}
      if(full>1) return false;
      const auto candidates=focus_candidates(space);
      if(space.focused && std::find(candidates.begin(),candidates.end(),*space.focused)==candidates.end()) return false;
      if(!space.focused && !candidates.empty()) return false;
    }
    return seen.size()==clients_.size();
  }
  // MANAGER API
private:
  std::map<int,Window> clients_;std::map<int,Workspace> workspaces_;int current_=1;
  Rect output_{0,0,1920,1080};Insets margins_;std::vector<Rule> rules_;
  Workspace& ws(){return workspaces_.at(current_);}
  const Workspace& ws() const {return workspaces_.at(current_);}
  Window& get(int id){auto it=clients_.find(id);if(it==clients_.end()) throw std::invalid_argument("unknown client");return it->second;}
  std::vector<int> focus_candidates(const Workspace& space) const {std::vector<int> ids;for(int id:space.windows){const auto& w=clients_.at(id);if(!w.minimized && !w.scratch){if(w.fullscreen) return {id};ids.push_back(id);}}return ids;}
  void repair_focus(Workspace& space){const auto ids=focus_candidates(space);if(!space.focused || std::find(ids.begin(),ids.end(),*space.focused)==ids.end()) space.focused=ids.empty()?std::nullopt:std::optional<int>(ids.back());}
  void apply_rules(int id){for(const auto& rule:rules_){auto& w=get(id);if(w.app_id!=rule.app_id) continue;if(rule.floating){w.floating=*rule.floating;w.frame=clamp_to(w.frame,work_area());}if(rule.workspace) move_to_workspace(id,*rule.workspace);}}
  // MANAGER PRIVATE
};
} // namespace hyprv4

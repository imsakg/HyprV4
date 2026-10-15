#include "hyprv4/manager.hpp"
#include "hyprv4/default_bindings.hpp"
#include "hyprv4/backend_state.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <limits>
#include <set>
namespace {
bool denied=false;
int errors(Display*,XErrorEvent* event){if(event->error_code==BadAccess) denied=true;return 0;}
int client_id(::Window window){if(window>static_cast<::Window>(std::numeric_limits<int>::max())) return 0;return static_cast<int>(window);}
}
int main(){
  Display* display=XOpenDisplay(nullptr);if(!display){std::cerr<<"cannot open X display\n";return 2;}
  const auto root=DefaultRootWindow(display);XSetErrorHandler(errors);
  XSelectInput(display,root,SubstructureRedirectMask|SubstructureNotifyMask|StructureNotifyMask);XSync(display,False);
  if(denied){std::cerr<<"another window manager owns this display\n";XCloseDisplay(display);return 2;}
  hyprv4::Manager manager;manager.set_output({0,0,DisplayWidth(display,DefaultScreen(display)),DisplayHeight(display,DefaultScreen(display))});
  for(int id=2;id<=9;++id) manager.add_workspace(id);
  const auto bindings=hyprv4::default_bindings();
  const auto wm_protocols=XInternAtom(display,"WM_PROTOCOLS",False),wm_delete=XInternAtom(display,"WM_DELETE_WINDOW",False);
  auto close_client=[&](int id){
    Atom* protocols=nullptr;int count=0;bool supported=false;
    if(XGetWMProtocols(display,static_cast<::Window>(id),&protocols,&count)) {for(int i=0;i<count;++i) if(protocols[i]==wm_delete) supported=true;if(protocols) XFree(protocols);}
    if(!supported){std::cerr<<"client has no WM_DELETE_WINDOW protocol: "<<id<<'\n';return;}
    XEvent message{};message.xclient.type=ClientMessage;message.xclient.window=static_cast<::Window>(id);message.xclient.message_type=wm_protocols;message.xclient.format=32;message.xclient.data.l[0]=static_cast<long>(wm_delete);message.xclient.data.l[1]=CurrentTime;
    XSendEvent(display,static_cast<::Window>(id),False,NoEventMask,&message);
  };
  for(const char* symbol:{"j","k","space","f","q","Escape","1","2","3","4","5","6","7","8","9"}){
    const auto code=XKeysymToKeycode(display,XStringToKeysym(symbol));if(!code) continue;
    for(unsigned shift:{0u,static_cast<unsigned>(ShiftMask)}) for(unsigned locks:{0u,static_cast<unsigned>(LockMask),static_cast<unsigned>(Mod2Mask),static_cast<unsigned>(LockMask|Mod2Mask)}) XGrabKey(display,code,Mod4Mask|shift|locks,root,True,GrabModeAsync,GrabModeAsync);
  }
  std::set<int> mapped;hyprv4::VisibilityLedger visibility;hyprv4::StackingOrder stacking;
  bool running=true;
  auto adopt=[&](::Window window){
    const int id=client_id(window);XWindowAttributes attributes{};
    if(!id || !XGetWindowAttributes(display,window,&attributes) || attributes.override_redirect) return;
    char* title=nullptr;XFetchName(display,window,&title);std::string name=title?title:"";if(title) XFree(title);
    if(!manager.find(id)) manager.manage(id,name);
    XSelectInput(display,window,PropertyChangeMask|EnterWindowMask);
  };
  auto apply=[&](){
    std::set<int> visible;std::vector<int> order;
    for(const auto& p:manager.placements()){
      visible.insert(p.id);order.push_back(p.id);const auto frame=hyprv4::content_rect(p.frame,manager.config().border);
      XSetWindowBorderWidth(display,static_cast<::Window>(p.id),static_cast<unsigned>(manager.config().border));
      XMoveResizeWindow(display,static_cast<::Window>(p.id),frame.x,frame.y,static_cast<unsigned>(std::max(1,frame.width)),static_cast<unsigned>(std::max(1,frame.height)));
      if(!mapped.count(p.id)){XMapWindow(display,static_cast<::Window>(p.id));mapped.insert(p.id);visibility.show(p.id);}
    }
    if(stacking.update(order)) for(int id:order) XRaiseWindow(display,static_cast<::Window>(id));
    for(auto it=mapped.begin();it!=mapped.end();){if(!visible.count(*it)){XUnmapWindow(display,static_cast<::Window>(*it));visibility.hide(*it);it=mapped.erase(it);}else ++it;}
    if(manager.focused() && visible.count(*manager.focused())) XSetInputFocus(display,static_cast<::Window>(*manager.focused()),RevertToPointerRoot,CurrentTime);
    XFlush(display);
  };
  ::Window returned_root,parent,*children=nullptr;unsigned count=0;
  if(XQueryTree(display,root,&returned_root,&parent,&children,&count)){
    for(unsigned i=0;i<count;++i){XWindowAttributes a{};if(XGetWindowAttributes(display,children[i],&a) && a.map_state==IsViewable) adopt(children[i]);}
    if(children) XFree(children);
  }
  apply();
  while(running){XEvent event{};XNextEvent(display,&event);
    try{
      if(event.type==MapRequest){adopt(event.xmaprequest.window);}
      else if(event.type==DestroyNotify){const int id=client_id(event.xdestroywindow.window);manager.unmanage(id);mapped.erase(id);visibility.forget(id);}
      else if(event.type==UnmapNotify){const int id=client_id(event.xunmap.window);if(event.xunmap.event==root){if(!visibility.acknowledge_unmap(id)){manager.unmanage(id);mapped.erase(id);visibility.forget(id);}}}
      else if(event.type==ConfigureRequest){const auto& e=event.xconfigurerequest;const int id=client_id(e.window);const auto* window=manager.find(id);
        if(window){if(window->floating){auto f=window->frame;if(e.value_mask&CWX) f.x=e.x;if(e.value_mask&CWY) f.y=e.y;if(e.value_mask&CWWidth) f.width=e.width;if(e.value_mask&CWHeight) f.height=e.height;manager.configure(id,f);}}
        else{XWindowChanges changes{};changes.x=e.x;changes.y=e.y;changes.width=e.width;changes.height=e.height;changes.border_width=e.border_width;changes.sibling=e.above;changes.stack_mode=e.detail;XConfigureWindow(display,e.window,static_cast<unsigned>(e.value_mask),&changes);}
      }
      else if(event.type==ConfigureNotify && event.xconfigure.window==root) manager.set_output({0,0,event.xconfigure.width,event.xconfigure.height});
      else if(event.type==EnterNotify && event.xcrossing.mode==NotifyNormal) manager.pointer_enter(event.xcrossing.x_root,event.xcrossing.y_root);
      else if(event.type==KeyPress){
        const char* name=XKeysymToString(XLookupKeysym(&event.xkey,0));if(!name) continue;
        const auto mask=event.xkey.state;hyprv4::ModifierMask normalized=0;
        if(mask&ControlMask) normalized|=hyprv4::modifiers::Ctrl;if(mask&ShiftMask) normalized|=hyprv4::modifiers::Shift;
        if(mask&Mod1Mask) normalized|=hyprv4::modifiers::Alt;if(mask&Mod4Mask) normalized|=hyprv4::modifiers::Super;
        bindings.dispatch(hyprv4::KeyChord::from_event(name,normalized),[&](const hyprv4::Binding& binding){
          const auto focused=manager.focused();
          if(binding.action=="cycle") manager.cycle_focus(std::stoi(binding.arguments.at(0)));
          else if(binding.action=="workspace") manager.switch_workspace(std::stoi(binding.arguments.at(0)));
          else if(binding.action=="move-focused" && focused) manager.move_to_workspace(*focused,std::stoi(binding.arguments.at(0)));
          else if(binding.action=="floating-focused" && focused) manager.toggle_floating(*focused);
          else if(binding.action=="fullscreen-focused" && focused) manager.set_fullscreen(*focused,!manager.find(*focused)->fullscreen);
          else if(binding.action=="close-focused" && focused) close_client(*focused);
          else if(binding.action=="quit") running=false;
        });
      }
      else continue; // Ignore notifications generated by our own configure/map requests.
      apply();
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';}
  }
  for(int id:visibility.hidden()) XMapWindow(display,static_cast<::Window>(id));
  XUngrabKey(display,AnyKey,AnyModifier,root);XSetInputFocus(display,PointerRoot,RevertToPointerRoot,CurrentTime);XCloseDisplay(display);return 0;
}

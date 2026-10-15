#include "hyprv4/manager.hpp"
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
  std::set<int> mapped,hidden;
  auto adopt=[&](::Window window){
    const int id=client_id(window);XWindowAttributes attributes{};
    if(!id || !XGetWindowAttributes(display,window,&attributes) || attributes.override_redirect) return;
    char* title=nullptr;XFetchName(display,window,&title);std::string name=title?title:"";if(title) XFree(title);
    if(!manager.find(id)) manager.manage(id,name);
    XSelectInput(display,window,PropertyChangeMask|EnterWindowMask);
  };
  auto apply=[&](){
    std::set<int> visible;
    for(const auto& p:manager.placements()){
      visible.insert(p.id);const auto frame=hyprv4::content_rect(p.frame,manager.config().border);
      XSetWindowBorderWidth(display,static_cast<::Window>(p.id),static_cast<unsigned>(manager.config().border));
      XMoveResizeWindow(display,static_cast<::Window>(p.id),frame.x,frame.y,static_cast<unsigned>(std::max(1,frame.width)),static_cast<unsigned>(std::max(1,frame.height)));
      if(!mapped.count(p.id)){XMapWindow(display,static_cast<::Window>(p.id));mapped.insert(p.id);}XRaiseWindow(display,static_cast<::Window>(p.id));
    }
    for(auto it=mapped.begin();it!=mapped.end();){if(!visible.count(*it)){XUnmapWindow(display,static_cast<::Window>(*it));hidden.insert(*it);it=mapped.erase(it);}else ++it;}
    if(manager.focused() && visible.count(*manager.focused())) XSetInputFocus(display,static_cast<::Window>(*manager.focused()),RevertToPointerRoot,CurrentTime);
    XFlush(display);
  };
  ::Window returned_root,parent,*children=nullptr;unsigned count=0;
  if(XQueryTree(display,root,&returned_root,&parent,&children,&count)){
    for(unsigned i=0;i<count;++i){XWindowAttributes a{};if(XGetWindowAttributes(display,children[i],&a) && a.map_state==IsViewable) adopt(children[i]);}
    if(children) XFree(children);
  }
  apply();
  while(true){XEvent event{};XNextEvent(display,&event);
    try{
      if(event.type==MapRequest){adopt(event.xmaprequest.window);}
      else if(event.type==DestroyNotify){const int id=client_id(event.xdestroywindow.window);manager.unmanage(id);mapped.erase(id);hidden.erase(id);}
      else if(event.type==UnmapNotify){const int id=client_id(event.xunmap.window);if(event.xunmap.event==root){if(!hidden.erase(id)){manager.unmanage(id);mapped.erase(id);}}}
      else if(event.type==ConfigureRequest){const auto& e=event.xconfigurerequest;const int id=client_id(e.window);const auto* window=manager.find(id);
        if(window){if(window->floating){auto f=window->frame;if(e.value_mask&CWX) f.x=e.x;if(e.value_mask&CWY) f.y=e.y;if(e.value_mask&CWWidth) f.width=e.width;if(e.value_mask&CWHeight) f.height=e.height;manager.configure(id,f);}}
        else{XWindowChanges changes{};changes.x=e.x;changes.y=e.y;changes.width=e.width;changes.height=e.height;changes.border_width=e.border_width;changes.sibling=e.above;changes.stack_mode=e.detail;XConfigureWindow(display,e.window,static_cast<unsigned>(e.value_mask),&changes);}
      }
      else if(event.type==ConfigureNotify && event.xconfigure.window==root) manager.set_output({0,0,event.xconfigure.width,event.xconfigure.height});
      // X11 EVENTS
      apply();
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';}
  }
}

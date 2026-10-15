#include "check.hpp"
#include "hyprv4/backend_state.hpp"
using namespace hyprv4;
int main(){StackingOrder order;CHECK(order.update({1,2}));CHECK(!order.update({1,2}));CHECK(order.update({2,1}));CHECK(!order.update({2,1}));CHECK(order.update({}));
VisibilityLedger visibility;visibility.hide(1);CHECK(visibility.acknowledge_unmap(1));CHECK(visibility.hidden().count(1));CHECK(!visibility.acknowledge_unmap(1));visibility.show(1);CHECK(visibility.hidden().empty());
visibility.hide(2);visibility.hide(2);CHECK(visibility.acknowledge_unmap(2));CHECK(visibility.acknowledge_unmap(2));CHECK(!visibility.acknowledge_unmap(2));visibility.forget(2);CHECK(visibility.hidden().empty());
auto framed=x11_geometry(Rect{10,20,100,80},2,false);CHECK(framed.x==10);CHECK(framed.y==20);CHECK(framed.width==96);CHECK(framed.border==2);
auto full=x11_geometry(Rect{0,0,1920,1080},2,true);CHECK(full.width==1920);CHECK(full.border==0);
// NEXT TEST
}

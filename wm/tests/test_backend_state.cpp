#include "check.hpp"
#include "hyprv4/backend_state.hpp"
using namespace hyprv4;
int main(){StackingOrder order;CHECK(order.update({1,2}));CHECK(!order.update({1,2}));CHECK(order.update({2,1}));CHECK(!order.update({2,1}));CHECK(order.update({}));
VisibilityLedger visibility;visibility.hide(1);CHECK(visibility.acknowledge_unmap(1));CHECK(visibility.hidden().count(1));CHECK(!visibility.acknowledge_unmap(1));visibility.show(1);CHECK(visibility.hidden().empty());
visibility.hide(2);visibility.hide(2);CHECK(visibility.acknowledge_unmap(2));CHECK(visibility.acknowledge_unmap(2));CHECK(!visibility.acknowledge_unmap(2));visibility.forget(2);CHECK(visibility.hidden().empty());
// NEXT TEST
}

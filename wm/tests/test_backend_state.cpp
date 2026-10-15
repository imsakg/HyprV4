#include "check.hpp"
#include "hyprv4/backend_state.hpp"
using namespace hyprv4;
int main(){StackingOrder order;CHECK(order.update({1,2}));CHECK(!order.update({1,2}));CHECK(order.update({2,1}));CHECK(!order.update({2,1}));CHECK(order.update({}));
// NEXT TEST
}

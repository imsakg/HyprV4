#include "hyprv4/config.hpp"
#include "check.hpp"

int main() {
    const hyprv4::Config defaults;
    CHECK(defaults.gap == 8);
    CHECK(defaults.outer_gap == 8);
    CHECK(defaults.border == 2);
    CHECK(defaults.master_count == 1);
    CHECK(defaults.ratio == 0.6);
    CHECK(defaults.workspace_count == 9);
    CHECK(!defaults.focus_follows_mouse);
    CHECK(defaults.terminal == "kitty");
    const auto empty = hyprv4::Config::parse("");
    CHECK(empty.gap == defaults.gap);
    CHECK(empty.terminal == defaults.terminal);
}

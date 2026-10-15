#include "hyprv4/config.hpp"
#include "check.hpp"
#include <limits>
#include <stdexcept>
#include <string>

static bool rejects(const std::string& text, std::size_t line = 1) {
    try {
        (void)hyprv4::Config::parse(text);
    } catch (const std::invalid_argument& error) {
        return std::string(error.what()).find("line " + std::to_string(line) + ":") !=
               std::string::npos;
    }
    return false;
}

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

    const auto integers = hyprv4::Config::parse(
        "gap=13\nouter_gap=21\nborder=4\nmaster_count=3\nworkspace_count=12\n");
    CHECK(integers.gap == 13);
    CHECK(integers.outer_gap == 21);
    CHECK(integers.border == 4);
    CHECK(integers.master_count == 3);
    CHECK(integers.workspace_count == 12);
    CHECK(hyprv4::Config::parse("gap=" + std::to_string(std::numeric_limits<int>::max())).gap ==
          std::numeric_limits<int>::max());
    for (const auto* value : {"", "x", "1.0", "2px", "+3", "999999999999999999999", "-999999999999999999999"}) {
        CHECK(rejects(std::string("gap=") + value));
    }
    CHECK(rejects("gap=10\nborder=oops", 2));

    for (const auto* key : {"gap", "outer_gap", "border"}) {
        CHECK(rejects(std::string(key) + "=-1"));
    }
    for (const auto* key : {"master_count", "workspace_count"}) {
        CHECK(rejects(std::string(key) + "=0"));
        CHECK(rejects(std::string(key) + "=-1"));
    }
    const auto zero = hyprv4::Config::parse("gap=0\nouter_gap=0\nborder=0");
    CHECK(zero.gap == 0 && zero.outer_gap == 0 && zero.border == 0);
    CHECK(rejects("gap=0\nmaster_count=0", 2));
}

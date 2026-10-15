#pragma once

#include <string>

namespace hyprv4 {

// Portable policy only: no backend, environment, or process side effects.
struct Config {
    int gap = 8;
    int outer_gap = 8;
    int border = 2;
    int master_count = 1;
    double ratio = 0.6;
    int workspace_count = 9;
    bool focus_follows_mouse = false;
    std::string terminal = "kitty";

    static Config parse(const std::string&) { return {}; }
};

}  // namespace hyprv4

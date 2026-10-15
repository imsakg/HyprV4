#include "hyprv4/keybindings.hpp"
#include "check.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>

static bool bad_chord(const std::string& text) {
    try { (void)hyprv4::KeyChord::parse(text); }
    catch (const std::invalid_argument&) { return true; }
    return false;
}

int main() {
    using hyprv4::KeyChord;
    CHECK(KeyChord::parse("A").key() == "a");
    CHECK(KeyChord::parse("  ReTuRn\t").key() == "enter");
    CHECK(KeyChord::parse("F12").to_string() == "f12");
    CHECK(KeyChord::parse("/").key() == "/");
    CHECK(KeyChord::parse("a") == KeyChord::parse("A"));
    CHECK(!(KeyChord::parse("a") == KeyChord::parse("b")));
    CHECK(KeyChord::parse("a") < KeyChord::parse("b"));
    CHECK(!(KeyChord::parse("A") < KeyChord::parse("a")));
    CHECK(KeyChord::parse(KeyChord::parse("F12").to_string()) == KeyChord::parse("f12"));

    CHECK(KeyChord::parse(" shift + SUPER + alt + cTrL + A ").to_string() ==
          "Ctrl+Alt+Super+Shift+a");
    std::array<std::string, 4> modifiers = {"Alt", "Ctrl", "Shift", "Super"};
    do {
        const auto text = modifiers[0] + "+" + modifiers[1] + "+" + modifiers[2] + "+" + modifiers[3] + "+A";
        CHECK(KeyChord::parse(text) == KeyChord::parse("Ctrl+Alt+Super+Shift+a"));
    } while (std::next_permutation(modifiers.begin(), modifiers.end()));
    CHECK(!(KeyChord::parse("Ctrl+a") == KeyChord::parse("a")));
    CHECK(!(KeyChord::parse("Ctrl+a") == KeyChord::parse("Alt+a")));
    CHECK(KeyChord::parse("Ctrl+a") < KeyChord::parse("Alt+a"));
    CHECK(KeyChord::parse("Ctrl+a").key() == "a");
    CHECK(KeyChord::parse(KeyChord::parse("super+shift+F2").to_string()) ==
          KeyChord::parse("Shift+Super+f2"));
    CHECK(bad_chord("Hyper+a"));

    for (const auto* alias : {"Control", "CTL"}) {
        CHECK(KeyChord::parse(std::string(alias) + "+A") == KeyChord::parse("Ctrl+a"));
    }
    for (const auto* alias : {"Option", "Mod1"}) {
        CHECK(KeyChord::parse(std::string(alias) + "+a") == KeyChord::parse("Alt+a"));
    }
    for (const auto* alias : {"Meta", "Win", "Windows", "Logo", "Mod4", "Cmd", "Command"}) {
        CHECK(KeyChord::parse(std::string(alias) + "+a") == KeyChord::parse("Super+a"));
    }
    for (const auto& alias : {std::pair{"Return", "enter"}, {"Esc", "escape"},
                              {"Spacebar", "space"}, {"Del", "delete"}, {"Ins", "insert"},
                              {"PgUp", "pageup"}, {"Prior", "pageup"},
                              {"PgDn", "pagedown"}, {"PgDown", "pagedown"}, {"Next", "pagedown"},
                              {"ArrowLeft", "left"}, {"ArrowRight", "right"},
                              {"ArrowUp", "up"}, {"ArrowDown", "down"}}) {
        CHECK(KeyChord::parse(std::string("Ctrl+") + alias.first) ==
              KeyChord::parse(std::string("Control+") + alias.second));
        CHECK(KeyChord::parse(alias.first).to_string() == alias.second);
    }
    CHECK(KeyChord::parse("Option+Command+Return").to_string() == "Alt+Super+enter");

    for (const auto* text : {"", "  \t", "Ctrl", "Control", "Ctrl+", "+a", "Ctrl++a",
                             "a+b", "Ctrl+a+b", "Ctrl+Alt", "Ctrl+Ctrl+a", "Ctrl+Control+a",
                             "Win+Super+a", "shift+SHIFT+a", "Ctrl,Alt+a", "CapsLock+a",
                             "Ctrl+two words", "Ctrl+a\nb", "Ctrl+\r", "Ctrl+é"}) {
        CHECK(bad_chord(text));
    }
    CHECK(bad_chord(std::string("Ctrl+ab") + '\0' + "cd"));
    CHECK(bad_chord(std::string("Ctrl+") + static_cast<char>(127)));
    for (const auto* text : {"0", "XF86AudioMute", "KP_Enter", "plus", "-", "=", ",", ".", "[", "]", "\\", ";", "'", "!"}) {
        CHECK(KeyChord::parse(KeyChord::parse(text).to_string()) == KeyChord::parse(text));
    }
}

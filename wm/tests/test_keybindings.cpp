#include "hyprv4/keybindings.hpp"
#include "check.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>

static bool bad_chord(const std::string& text) {
    try { (void)hyprv4::KeyChord::parse(text); }
    catch (const std::invalid_argument&) { return true; }
    return false;
}

int main() {
    using hyprv4::KeyChord;
    CHECK(KeyChord::parse("A").key() == "a");
    CHECK(KeyChord::parse("  ReTuRn\t").key() == "return");
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
}

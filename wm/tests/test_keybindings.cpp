#include "hyprv4/keybindings.hpp"
#include "check.hpp"
#include <string>

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
}

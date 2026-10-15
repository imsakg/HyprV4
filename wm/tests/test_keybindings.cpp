#include "hyprv4/keybindings.hpp"
#include "check.hpp"
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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

    namespace mods = hyprv4::modifiers;
    CHECK(KeyChord::from_event("Return", mods::Super | mods::CapsLock | mods::NumLock) ==
          KeyChord::parse("Super+enter"));
    for (hyprv4::ModifierMask mask = 0; mask < 128; ++mask) {
        const auto event = KeyChord::from_event("A", mask);
        CHECK(event.modifiers() == (mask & mods::Active));
        CHECK(event.key() == "a");
        CHECK(event == KeyChord::from_event("a", mask & mods::Active));
        CHECK(KeyChord::parse(event.to_string()) == event);
    }
    CHECK(KeyChord::from_event("A") == KeyChord::parse("a"));
    CHECK(!(KeyChord::from_event("A") == KeyChord::parse("Shift+a")));
    CHECK(KeyChord::from_event("+", mods::Shift) == KeyChord::parse("Shift+plus"));
    CHECK(KeyChord::from_event(" ", mods::Ctrl) == KeyChord::parse("Ctrl+space"));
    bool unknown_event_mask = false;
    try { (void)KeyChord::from_event("a", hyprv4::ModifierMask{1} << 31); }
    catch (const std::invalid_argument&) { unknown_event_mask = true; }
    CHECK(unknown_event_mask);
    for (const auto* key : {"", "Ctrl+a", "Ctrl", "two words", "a\nb"}) {
        bool invalid_event = false;
        try { (void)KeyChord::from_event(key, mods::ScrollLock); }
        catch (const std::invalid_argument&) { invalid_event = true; }
        CHECK(invalid_event);
    }

    hyprv4::KeyBindings bindings;
    CHECK(bindings.empty() && bindings.size() == 0);
    CHECK(bindings.lookup("Super+Return") == nullptr);
    std::string action = "launch";
    std::vector<std::string> arguments = {"kitty", "--title=dev desk", "", std::string("a\0b", 3)};
    bindings.bind("Super+Return", action, arguments);
    action = "changed";
    arguments[0] = "changed";
    CHECK(!bindings.empty() && bindings.size() == 1);
    const auto* launch = bindings.lookup(KeyChord::from_event("Enter", mods::Super | mods::CapsLock));
    CHECK(launch != nullptr);
    CHECK(launch->action == "launch");
    CHECK(launch->arguments == std::vector<std::string>({"kitty", "--title=dev desk", "", std::string("a\0b", 3)}));
    CHECK(bindings.lookup("Return") == nullptr);
    bindings.bind(KeyChord::parse("Ctrl+Shift+x"), "workspace", {"3"});
    CHECK(bindings.lookup("shift+control+X")->action == "workspace");
    CHECK(bindings.lookup("shift+control+X")->arguments == std::vector<std::string>({"3"}));
    bindings.bind("a", "noop");
    CHECK(bindings.lookup("a")->arguments.empty());
    CHECK(bindings.size() == 3);
    for (const auto* invalid : {"", "  ", "launch app", "\tlaunch", "launch\n"}) {
        bool failed = false;
        try { bindings.bind("Super+z", invalid); }
        catch (const std::invalid_argument&) { failed = true; }
        CHECK(failed && bindings.lookup("Super+z") == nullptr && bindings.size() == 3);
    }
    bool malformed_bind = false;
    try { bindings.bind("Ctrl++a", "noop"); }
    catch (const std::invalid_argument&) { malformed_bind = true; }
    CHECK(malformed_bind && bindings.size() == 3);
}

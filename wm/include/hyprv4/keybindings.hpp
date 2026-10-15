#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace hyprv4 {

using ModifierMask = std::uint32_t;
namespace modifiers {
inline constexpr ModifierMask Ctrl = 1u << 0;
inline constexpr ModifierMask Alt = 1u << 1;
inline constexpr ModifierMask Super = 1u << 2;
inline constexpr ModifierMask Shift = 1u << 3;
inline constexpr ModifierMask CapsLock = 1u << 4;
inline constexpr ModifierMask NumLock = 1u << 5;
inline constexpr ModifierMask ScrollLock = 1u << 6;
inline constexpr ModifierMask Active = Ctrl | Alt | Super | Shift;
inline constexpr ModifierMask Locks = CapsLock | NumLock | ScrollLock;
}  // namespace modifiers

// Symbolic policy keys, not backend keycodes or a keyboard-layout implementation.
class KeyChord {
public:
    static KeyChord parse(const std::string& text) {
        ModifierMask mask = 0;
        std::size_t start = 0;
        for (auto plus = text.find('+'); plus != std::string::npos; plus = text.find('+', start)) {
            const auto token = lower(trim(text.substr(start, plus - start)));
            const auto bit = modifier(token);
            if (bit == 0) throw std::invalid_argument("unknown modifier: " + token);
            if (mask & bit) throw std::invalid_argument("duplicate modifier: " + token);
            mask |= bit;
            start = plus + 1;
        }
        return KeyChord(mask, canonical_key(text.substr(start)));
    }

    // Callers translate backend modifier bits into this library's mask first.
    static KeyChord from_event(const std::string& key, ModifierMask mask = 0) {
        if (mask & ~(modifiers::Active | modifiers::Locks)) {
            throw std::invalid_argument("event contains unknown modifier bits");
        }
        const auto symbol = key == "+" ? "plus" : key == " " ? "space" : key;
        return KeyChord(mask & modifiers::Active, canonical_key(symbol));
    }

    const std::string& key() const noexcept { return key_; }
    ModifierMask modifiers() const noexcept { return modifiers_; }
    std::string to_string() const {
        std::string result;
        if (modifiers_ & modifiers::Ctrl) result += "Ctrl+";
        if (modifiers_ & modifiers::Alt) result += "Alt+";
        if (modifiers_ & modifiers::Super) result += "Super+";
        if (modifiers_ & modifiers::Shift) result += "Shift+";
        return result + key_;
    }

    friend bool operator==(const KeyChord& a, const KeyChord& b) {
        return a.modifiers_ == b.modifiers_ && a.key_ == b.key_;
    }
    friend bool operator<(const KeyChord& a, const KeyChord& b) {
        if (a.modifiers_ != b.modifiers_) return a.modifiers_ < b.modifiers_;
        return a.key_ < b.key_;
    }

private:
    ModifierMask modifiers_;
    std::string key_;

    KeyChord(ModifierMask mask, std::string key) : modifiers_(mask), key_(std::move(key)) {}

    static ModifierMask modifier(const std::string& token) {
        if (token == "ctrl" || token == "control" || token == "ctl") return modifiers::Ctrl;
        if (token == "alt" || token == "option" || token == "mod1") return modifiers::Alt;
        if (token == "super" || token == "meta" || token == "win" || token == "windows" ||
            token == "logo" || token == "mod4" || token == "cmd" || token == "command") {
            return modifiers::Super;
        }
        if (token == "shift") return modifiers::Shift;
        return 0;
    }

    static std::string canonical_key(const std::string& text) {
        const auto token = lower(trim(text));
        if (token.empty() || modifier(token) != 0) {
            throw std::invalid_argument("chord requires one non-modifier key");
        }
        const bool punctuation = token.size() == 1 && token[0] >= '!' &&
                                 token[0] <= '~' && token[0] != '+';
        if (!punctuation) {
            for (char byte : token) {
                if (!((byte >= 'a' && byte <= 'z') || (byte >= '0' && byte <= '9') || byte == '_')) {
                    throw std::invalid_argument("key must be an ASCII identifier or single punctuation mark");
                }
            }
        }
        if (token == "return") return "enter";
        if (token == "esc") return "escape";
        if (token == "spacebar") return "space";
        if (token == "del") return "delete";
        if (token == "ins") return "insert";
        if (token == "pgup" || token == "prior") return "pageup";
        if (token == "pgdn" || token == "pgdown" || token == "next") return "pagedown";
        if (token == "arrowleft") return "left";
        if (token == "arrowright") return "right";
        if (token == "arrowup") return "up";
        if (token == "arrowdown") return "down";
        return token;
    }

    static std::string trim(const std::string& text) {
        const auto start = text.find_first_not_of(" \t");
        if (start == std::string::npos) return {};
        return text.substr(start, text.find_last_not_of(" \t") - start + 1);
    }

    static std::string lower(std::string text) {
        for (char& byte : text) {
            if (byte >= 'A' && byte <= 'Z') byte = static_cast<char>(byte - 'A' + 'a');
        }
        return text;
    }
};

struct Binding {
    std::string action;
    std::vector<std::string> arguments;
};

class KeyBindings {
public:
    void bind(const KeyChord& chord, std::string action, std::vector<std::string> arguments = {}) {
        if (action.empty()) throw std::invalid_argument("action must not be empty");
        for (unsigned char byte : action) {
            if (byte <= 32 || byte == 127) {
                throw std::invalid_argument("action name must not contain whitespace or controls");
            }
        }
        const auto inserted = bindings_.emplace(chord, Binding{std::move(action), std::move(arguments)});
        if (!inserted.second) throw std::invalid_argument("duplicate binding: " + chord.to_string());
    }

    void bind(const std::string& chord, std::string action, std::vector<std::string> arguments = {}) {
        bind(KeyChord::parse(chord), std::move(action), std::move(arguments));
    }

    const Binding* lookup(const KeyChord& chord) const {
        const auto found = bindings_.find(chord);
        return found == bindings_.end() ? nullptr : &found->second;
    }

    const Binding* lookup(const std::string& chord) const { return lookup(KeyChord::parse(chord)); }

    // Dispatch a snapshot so a callback may safely change the owning table.
    template <typename Handler>
    bool dispatch(const KeyChord& chord, Handler&& handler) const {
        const auto* found = lookup(chord);
        if (!found) return false;
        const Binding snapshot = *found;
        std::forward<Handler>(handler)(snapshot);
        return true;
    }

    template <typename Handler>
    bool dispatch(const std::string& chord, Handler&& handler) const {
        return dispatch(KeyChord::parse(chord), std::forward<Handler>(handler));
    }

    bool unbind(const KeyChord& chord) { return bindings_.erase(chord) != 0; }
    bool unbind(const std::string& chord) { return unbind(KeyChord::parse(chord)); }

    std::size_t size() const noexcept { return bindings_.size(); }
    bool empty() const noexcept { return bindings_.empty(); }

private:
    std::map<KeyChord, Binding> bindings_;
};

}  // namespace hyprv4

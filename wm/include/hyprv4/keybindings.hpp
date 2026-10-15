#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace hyprv4 {

using ModifierMask = std::uint32_t;
namespace modifiers {
inline constexpr ModifierMask Ctrl = 1u << 0;
inline constexpr ModifierMask Alt = 1u << 1;
inline constexpr ModifierMask Super = 1u << 2;
inline constexpr ModifierMask Shift = 1u << 3;
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

    const std::string& key() const noexcept { return key_; }
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

}  // namespace hyprv4

#pragma once

#include <string>
#include <utility>

namespace hyprv4 {

// Symbolic policy keys, not backend keycodes or a keyboard-layout implementation.
class KeyChord {
public:
    static KeyChord parse(const std::string& text) { return KeyChord(lower(trim(text))); }

    const std::string& key() const noexcept { return key_; }
    std::string to_string() const { return key_; }

    friend bool operator==(const KeyChord& a, const KeyChord& b) { return a.key_ == b.key_; }
    friend bool operator<(const KeyChord& a, const KeyChord& b) { return a.key_ < b.key_; }

private:
    std::string key_;

    explicit KeyChord(std::string key) : key_(std::move(key)) {}

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

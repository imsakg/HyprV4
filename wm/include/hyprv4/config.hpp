#pragma once

#include <charconv>
#include <cmath>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

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

    // Full replacement from defaults; parse errors never mutate the live object.
    void reload(const std::string& text) { *this = parse(text); }

    static Config parse(const std::string& text) {
        Config result;
        std::istringstream input(text);
        std::string line;
        std::size_t number = 0;
        std::set<std::string> seen;
        while (std::getline(input, line)) {
            ++number;
            line = trim(line.substr(0, line.find('#')));
            if (line.empty()) continue;
            const auto equals = line.find('=');
            if (equals == std::string::npos) fail(number, "expected key=value");
            const auto key = trim(line.substr(0, equals));
            const auto value = trim(line.substr(equals + 1));
            if (!seen.insert(key).second) fail(number, "duplicate key: " + key);
            if (key == "gap") result.gap = integer(value, number, 0);
            else if (key == "outer_gap") result.outer_gap = integer(value, number, 0);
            else if (key == "border") result.border = integer(value, number, 0);
            else if (key == "master_count") result.master_count = integer(value, number, 1);
            else if (key == "workspace_count") result.workspace_count = integer(value, number, 1);
            else if (key == "ratio") result.ratio = fraction(value, number);
            else if (key == "focus_follows_mouse") result.focus_follows_mouse = boolean(value, number);
            else if (key == "terminal") result.terminal = terminal_text(value, number);
            else fail(number, "unknown key: " + key);
        }
        return result;
    }

private:
    static std::string trim(const std::string& text) {
        constexpr auto whitespace = " \t\r\v\f";
        const auto start = text.find_first_not_of(whitespace);
        if (start == std::string::npos) return {};
        return text.substr(start, text.find_last_not_of(whitespace) - start + 1);
    }

    static std::string terminal_text(const std::string& value, std::size_t line) {
        if (value.empty()) fail(line, "terminal must not be empty");
        for (unsigned char byte : value) {
            if (byte < 32 || byte == 127 || byte == '#') {
                fail(line, "terminal must be single-line text without controls or #");
            }
        }
        return value;
    }

    static bool boolean(const std::string& value, std::size_t line) {
        if (value == "true") return true;
        if (value == "false") return false;
        fail(line, "expected true or false");
    }

    static double fraction(const std::string& value, std::size_t line) {
        std::istringstream input(value);
        input.imbue(std::locale::classic());
        double result = 0.0;
        input >> std::noskipws >> result;
        if (!input || input.peek() != std::char_traits<char>::eof() ||
            !std::isfinite(result) || result <= 0.0 || result >= 1.0) {
            fail(line, "ratio must be finite and strictly between 0 and 1");
        }
        return result;
    }

    [[noreturn]] static void fail(std::size_t line, const std::string& reason) {
        throw std::invalid_argument("line " + std::to_string(line) + ": " + reason);
    }

    static int integer(const std::string& value, std::size_t line, int minimum) {
        int result = 0;
        const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
        if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size()) {
            fail(line, "expected a decimal integer");
        }
        if (result < minimum) fail(line, "integer must be at least " + std::to_string(minimum));
        return result;
    }
};

}  // namespace hyprv4

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

static bool same(const hyprv4::Config& a, const hyprv4::Config& b) {
    return a.gap == b.gap && a.outer_gap == b.outer_gap && a.border == b.border &&
           a.master_count == b.master_count && a.ratio == b.ratio &&
           a.workspace_count == b.workspace_count &&
           a.focus_follows_mouse == b.focus_follows_mouse && a.terminal == b.terminal;
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

    CHECK(hyprv4::Config::parse("ratio=0.75").ratio == 0.75);
    CHECK(hyprv4::Config::parse("ratio=2.5e-1").ratio == 0.25);
    CHECK(hyprv4::Config::parse("ratio=.5").ratio == 0.5);
    for (const auto* value : {"", "0", "1", "-0.1", "1.1", "nan", "NaN", "inf", "-inf", "1e9999", "1e-9999", "0.5x", "0,5"}) {
        CHECK(rejects(std::string("ratio=") + value));
    }
    CHECK(rejects("gap=0\nratio=1", 2));

    CHECK(hyprv4::Config::parse("focus_follows_mouse=true").focus_follows_mouse);
    CHECK(!hyprv4::Config::parse("focus_follows_mouse=false").focus_follows_mouse);
    for (const auto* value : {"", "True", "FALSE", "1", "0", "yes", "truefalse"}) {
        CHECK(rejects(std::string("focus_follows_mouse=") + value));
    }
    CHECK(rejects("gap=0\nfocus_follows_mouse=yes", 2));

    CHECK(hyprv4::Config::parse("terminal=foot").terminal == "foot");
    CHECK(hyprv4::Config::parse("terminal=/opt/term --title=dev desk").terminal ==
          "/opt/term --title=dev desk");
    CHECK(rejects("terminal="));
    CHECK(rejects(std::string("terminal=ki") + '\0' + "tty"));
    CHECK(rejects("terminal=kitty\t--hold"));
    CHECK(rejects("gap=0\nterminal=", 2));

    const auto spaced = hyprv4::Config::parse(
        "  # policy defaults\r\n\t\r\n  gap \t= 11  # inner gap\r\n"
        " outer_gap= 12\r\n border=0 \n master_count = 2 \n ratio = .7 \n"
        " workspace_count=4\n focus_follows_mouse = true # enable\n"
        " terminal = foot --title=a desk  # literal command\n");
    CHECK(spaced.gap == 11 && spaced.outer_gap == 12 && spaced.border == 0);
    CHECK(spaced.master_count == 2 && spaced.ratio == 0.7 && spaced.workspace_count == 4);
    CHECK(spaced.focus_follows_mouse && spaced.terminal == "foot --title=a desk");
    CHECK(hyprv4::Config::parse("  # nothing\r\n\t\n").gap == 8);
    CHECK(rejects("# header\n\n gap = invalid", 3));
    CHECK(rejects("terminal= # no command"));

    for (const auto* text : {"unknown=1", "Gap=1", "gap 8", "=8", "gap==8", "[layout]"}) {
        CHECK(rejects(text));
    }
    for (const auto* key : {"gap", "outer_gap", "border", "master_count", "ratio",
                            "workspace_count", "focus_follows_mouse", "terminal"}) {
        const std::string value = std::string(key) == "terminal" ? "foot" :
                                  std::string(key) == "focus_follows_mouse" ? "true" :
                                  std::string(key) == "ratio" ? "0.7" : "2";
        CHECK(rejects(std::string(key) + "=" + value + "\n# duplicate\n  " + key + " = " + value, 3));
    }
    CHECK(rejects("# heading\n\nunknown=1", 3));
    CHECK(rejects("gap=1\nmissing equals", 2));

    auto live = hyprv4::Config::parse(
        "gap=3\nouter_gap=4\nborder=5\nmaster_count=2\nratio=.7\n"
        "workspace_count=6\nfocus_follows_mouse=true\nterminal=foot");
    const auto saved = live;
    for (const auto* bad : {"gap=99\nratio=1", "terminal=other\nunknown=1",
                            "gap=99\ngap=100", "workspace_count=0"}) {
        bool failed = false;
        try { live.reload(bad); }
        catch (const std::invalid_argument&) { failed = true; }
        CHECK(failed && same(live, saved));
    }
    live.reload("gap=42");
    CHECK(live.gap == 42);
    CHECK(live.terminal == "kitty" && live.ratio == 0.6 && !live.focus_follows_mouse);
    live.reload("");
    CHECK(same(live, defaults));
}

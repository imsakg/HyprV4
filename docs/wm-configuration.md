# Portable WM policy configuration

`hyprv4/config.hpp` is a header-only C++17 configuration value type. It is independent of Hyprland, Wayland, X11, keyboard libraries, the environment, and filesystem locations. It does not start a compositor or execute a terminal command.

## API

```cpp
#include "hyprv4/config.hpp"

hyprv4::Config config; // Defaults below.
config = hyprv4::Config::parse("gap=12\nratio=0.65\n");
config.reload("terminal=foot\n"); // Full replacement, not a merge.
config.validate();                // Check directly edited public fields.
const std::string text = config.serialize();
```

- `static Config parse(const std::string&)` starts from defaults and returns a new value. Invalid input throws `std::invalid_argument` with a one-based physical `line N:` prefix, including blank and comment lines in the count.
- `void reload(const std::string&)` parses before replacing the existing value. On a parse error every field remains unchanged. Omitted options reset to defaults. This is exception atomicity, not cross-thread synchronization; callers must synchronize shared access.
- `void validate() const` rejects invalid directly edited values with `std::invalid_argument`.
- `std::string serialize() const` validates then emits all eight keys in the order listed below, with a final newline, lowercase booleans, and classic-locale roundtrip double precision. Parsing the result preserves every field exactly, although `0.6` may print with additional digits. Comments and original formatting are not retained.

## Defaults and limits

- `gap=8`: nonnegative inner spacing, in policy coordinate units.
- `outer_gap=8`: nonnegative outer spacing.
- `border=2`: nonnegative border width.
- `master_count=1`: positive master-window count.
- `ratio=0.6`: finite master-area fraction, strictly greater than zero and less than one.
- `workspace_count=9`: positive workspace count.
- `focus_follows_mouse=false`: exactly `true` or `false`.
- `terminal=kitty`: nonempty single-line literal text.

Integers are base-10 C++ `int` values, allowing a leading minus but no leading plus. Negative spacing and nonpositive counts are invalid; integer overflow is rejected. Ratios use a dot decimal separator and may use scientific notation; NaN, infinity, underflow to zero, overflow, trailing junk, and comma decimals are rejected independently of the process locale.

## File grammar

Each nonempty, noncomment line is `key=value`. Keys are case sensitive. Unknown keys, duplicate keys (even if the values agree), missing separators, and invalid values are errors. The first `=` separates the key from its value; additional `=` characters are literal terminal text but invalid in numeric/boolean values.

ASCII space, tab, carriage return, vertical tab, and form feed around a line, key, or value are trimmed. LF and CRLF files work. `#` starts a comment anywhere on the line. There is no quoting or escaping grammar: quotes are literal characters, and `#` cannot be part of a terminal value. Terminal text may contain interior spaces and equals signs, but not ASCII control characters or DEL. Directly edited terminal values with surrounding whitespace are rejected by validation/serialization rather than silently changed.

The terminal option is data, not a shell parser. An integration decides how to interpret it; this library performs no command expansion, process launch, config-file I/O, or filesystem watching. Keybindings are a separate in-memory API, not extra keys accepted by this parser; see [WM keybindings](wm-keybindings.md).

A complete default file is provided at [`wm/examples/core.conf`](../wm/examples/core.conf).

## Focused verification

Run from the repository root, choosing an output location outside the source tree:

```sh
clang++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
  -Iwm/include -Iwm/tests wm/tests/test_config.cpp -o "$TMPDIR/hyprv4-test-config"
"$TMPDIR/hyprv4-test-config"
```

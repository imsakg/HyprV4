# Portable WM keybindings

`hyprv4/keybindings.hpp` provides header-only C++17 symbolic chord normalization and an in-memory action table. It contains no Wayland, Hyprland, X11, xkbcommon, event-loop, or shell dependencies. A backend translates its key events into these policy values and decides what the returned action means.

## Example

```cpp
#include "hyprv4/config.hpp"
#include "hyprv4/keybindings.hpp"
#include <cassert>
#include <string>

int main() {
    const auto config = hyprv4::Config::parse("terminal=foot\n");
    hyprv4::KeyBindings keys;
    keys.bind("Super+Return", "launch", {config.terminal});
    keys.bind("Ctrl+Shift+2", "workspace", {"2"});

    namespace mods = hyprv4::modifiers;
    const auto event = hyprv4::KeyChord::from_event("Enter", mods::Super | mods::CapsLock);
    std::string requested_action;
    const bool matched = keys.dispatch(event, [&](const hyprv4::Binding& binding) {
        requested_action = binding.action;
        assert(binding.arguments.at(0) == "foot");
        // Route the data to a policy consumer here; no command is executed.
    });
    assert(matched && requested_action == "launch");
    assert(keys.unbind("Meta+Return"));
    assert(keys.lookup(event) == nullptr);
}
```

Configuration and keybindings are separate APIs. The configuration parser does not accept binding directives. There are no default bindings or predefined action names; the caller installs both.

## Chord API and grammar

- `static KeyChord parse(const std::string&)` parses zero or more modifiers followed by exactly one non-modifier key, separated with `+`. Invalid or ambiguous syntax throws `std::invalid_argument`.
- `static KeyChord from_event(const std::string& key, ModifierMask mask = 0)` normalizes a single event key and strips recognized lock-state bits. It does not parse a chord string in the key argument.
- `const std::string& key() const noexcept` returns the canonical key name.
- `ModifierMask modifiers() const noexcept` returns only active modifier bits.
- `std::string to_string() const` produces modifiers in `Ctrl+Alt+Super+Shift` order followed by the lowercase key. Parsing this representation preserves identity.
- `operator==` compares normalized mask and key. `operator<` orders first by the numeric active mask, then by the lowercase key. Chords are value types with no default constructor or mutable public fields.

ASCII case is folded independently of the process locale. Spaces and tabs around tokens are ignored. A symbolic key is a nonempty ASCII identifier consisting of letters, digits, or underscores (for example `F12`, `KP_Enter`, or `XF86AudioMute`), or one printable ASCII punctuation character other than `+`. Identifiers are syntactically checked, not checked against a backend-specific key database. Write `plus` for the plus key and `space` for a space key. Event normalization additionally accepts a literal `"+"` or `" "` and converts it to the named key.

Empty chords, empty tokens, modifier-only chords, repeated modifiers (including aliases of the same modifier), unknown modifier prefixes, multiple keys, embedded whitespace/control characters, and non-ASCII symbolic names are errors. `Ctrl++` is not the plus key. Lock names are not textual modifiers, although a bare `CapsLock` can name the physical lock key.

### Modifier aliases

All aliases are case insensitive:

- `Ctrl`: `Control`, `Ctl`.
- `Alt`: `Option`, `Mod1`.
- `Super`: `Meta`, `Win`, `Windows`, `Logo`, `Mod4`, `Cmd`, `Command`.
- `Shift`: no additional aliases.

These are policy synonyms, not claims about a particular platform's hardware mapping. For example `Option+Command+Return` canonicalizes to `Alt+Super+enter`.

### Key aliases

- `Return` → `enter`.
- `Esc` → `escape`.
- `Spacebar` → `space`.
- `Del` → `delete`; `Ins` → `insert`.
- `PgUp`, `Prior` → `pageup`.
- `PgDn`, `PgDown`, `Next` → `pagedown`.
- `ArrowLeft`, `ArrowRight`, `ArrowUp`, `ArrowDown` → `left`, `right`, `up`, `down`.

Other identifiers retain their lowercase spelling. Keypad keys are not collapsed into non-keypad keys.

## Event modifier masks

`hyprv4::ModifierMask` is `std::uint32_t`. `hyprv4::modifiers` defines independent bit flags:

- `Ctrl`, `Alt`, `Super`, `Shift`: bits 0 through 3.
- `CapsLock`, `NumLock`, `ScrollLock`: bits 4 through 6, ignored for matching.
- `Active`: the union of the four active bits.
- `Locks`: the union of the three lock bits.

Translate backend flags explicitly before calling `from_event`: these values are **not** raw Wayland, X11, xkb, or OS flags. Unknown bits are rejected rather than silently discarded. Uppercase key text does not implicitly add Shift. The backend supplies actual Shift state; lock-state masking changes neither the supplied active mask nor the backend's responsibility for choosing a symbolic key. Non-ASCII text input, physical scan-code mapping, keyboard layouts, and composed text are outside this core.

## Binding table API

`Binding` owns `std::string action` and `std::vector<std::string> arguments`. Action names must be nonempty and contain no ASCII whitespace, control bytes, or DEL. Their meaning is otherwise unrestricted. Arguments are opaque strings, including empty strings or embedded NUL bytes; the core does not split, expand, validate, or execute them.

`KeyBindings` supports both a typed `const KeyChord&` and textual `const std::string&` overload for:

- `void bind(chord, std::string action, std::vector<std::string> arguments = {})`: stores owning copies/moved values. Canonical duplicate chords throw `std::invalid_argument`, even if the action agrees. The original record remains unchanged. Replace deliberately with `unbind` followed by `bind`.
- `const Binding* lookup(chord) const`: exact-mask matching; returns `nullptr` if absent. Extra active modifiers are not ignored. The pointer remains valid while its record and owning table remain alive, unless the record is erased or the table is assigned/moved. Inserting another record does not invalidate it.
- `template <typename Handler> bool dispatch(chord, Handler&& handler) const`: synchronously invokes `handler(const Binding&)` exactly once on a match and returns `true`; returns `false` without invoking on a miss. A local record snapshot permits callbacks to unbind or replace the current record. The snapshot reference is valid only during the callback. Move-only handlers are supported; callback exceptions propagate.
- `bool unbind(chord)`: removes only the exact normalized chord, returning whether a record was removed.

`std::size_t size() const noexcept` and `bool empty() const noexcept` report table occupancy. All textual overloads apply the same strict chord parser, so malformed text throws rather than being treated as an unbound chord. Tables are ordinary value types; callers synchronize cross-thread access. No press/release distinction, repeat suppression, event capture, scheduling, command launch, or compositor action is performed by this layer.

## Focused verification

From the repository root:

```sh
clang++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
  -Iwm/include -Iwm/tests wm/tests/test_keybindings.cpp -o "$TMPDIR/hyprv4-test-keybindings"
"$TMPDIR/hyprv4-test-keybindings"
```

See [configuration](wm-configuration.md) for the separate policy-value parser.

# C++ window-management core

HyprV4 originally contains Hyprland dotfiles and an Arch installer. `wm/` is a separate C++17 policy engine plus an opt-in experimental Xlib backend. It does **not** replace Hyprland or implement a Wayland compositor. Existing dotfiles/install behavior is unchanged.

## Build and run without a display

Requirements: CMake 3.16+, C++17 compiler with floating-point `std::from_chars` support (tested AppleClang 21), optionally Ninja. No dependencies are fetched.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
build/wm/hyprv4-core --config wm/examples/core.conf < wm/examples/session.commands
```

The command runner writes `ok` for successful actions, `unbound` for unmatched keys, and JSON for `state`. Empty lines and comments are ignored. Exit codes: 0 success, 1 one or more command errors, 2 startup/configuration/input I/O failure. Command errors are reported with line numbers and processing continues. No command invokes a shell or spawns the configured terminal.

## Engine boundaries

- `geometry.hpp`: validated half-open integer rectangles, checked translation, clipping, floating clamps, panel reservations.
- `layout.hpp`: master/stack, monocle, grid and bottom-stack; exact split pixel accounting; independent inner/outer gaps; defensive window-count limits.
- `manager.hpp`: unique client lifecycle, per-workspace focus/order/config, transfers, floating configure/move/resize, fullscreen exclusivity, minimize, urgency, scratchpad, application-ID rules, pointer focus, close requests and state invariant checks.
- `config.hpp`: strict transactional configuration; see [configuration](wm-configuration.md).
- `keybindings.hpp`: symbolic chord normalization, lock masking and collision-safe dispatch; see [keybindings](wm-keybindings.md).
- `commands.hpp`: shell-free quoted argument grammar, strict arity/integer validation, bounded binding recursion, deterministic active-workspace JSON snapshots.
- `backend_state.hpp`: change-only stacking reconciliation, persistent hidden-client tracking, correct X11 outer-border geometry.

Manager state has a **single output**, with arbitrary workspace IDs. `workspace_count` is a configuration preference; workspace creation is explicit in the policy API. Configuration replacement affects the active workspace; newly created workspaces inherit its config. Fullscreen covers the entire output, bypassing reserved dock margins. Floating windows are stacked and hit-tested above tiled clients. Close requests do not invent destruction: the backend owns real client shutdown.

The library has no display, process or filesystem side effects; callers translate OS events and apply returned placements. Title/app-ID matching uses exact strings, not regex. Scratchpad restore targets the current workspace. All state is in memory; JSON is an observation, not a persistence format. There is no IPC daemon or multi-output/RandR support.

## Script command reference

- `manage ID [TITLE [APP_ID]]`, `unmanage ID`, `focus ID`, `cycle DELTA`
- `workspace-add ID`, `workspace ID`, `move CLIENT_ID WORKSPACE_ID`
- `layout master|monocle|grid|bottom`
- `floating ID`, `configure ID X Y WIDTH HEIGHT`
- `fullscreen ID BOOL`, `minimize ID BOOL`, `close ID`
- `config TEXT`: replace active-workspace policy from the quoted complete configuration text
- `bind CHORD ACTION [ARGUMENTS...]`, `unbind CHORD`, `key CHORD`
- `state`: JSON containing workspace, focused ID/null and visible window IDs/titles/frames

Booleans are `true`/`false` or `1`/`0` in commands, but config files require `true`/`false`. Use single/double quotes for spaces; backslash escapes outside single quotes. `#` begins an unquoted comment. For multiline configuration, use `--config FILE` or the C++ `CommandRunner::reload(text)` API rather than a stdin line.

## Experimental X11 backend

Build with `-DHYPRV4_BUILD_X11=ON`; requires pkg-config and Xlib development headers/libraries. Target: `build/wm/hyprv4-x11`. The backend adopts viewable non-override-redirect windows, handles map/destroy/unmap/configure requests, and applies policy focus/geometry. It checks for an existing WM and refuses root ownership on `BadAccess`. It uses WM_DELETE_WINDOW when available and never forcibly kills clients.

**Use only on a disposable nested X server**, e.g. `DISPLAY=:99 build/wm/hyprv4-x11` after starting Xvfb/Xephyr yourself. Never replace your active desktop WM for testing. The backend was compiled and its no-display error path exercised on macOS, but a live nested X server was unavailable: display event integration is not certified.

Defaults: Super+j/k cycle, Super+space toggle floating, Super+f fullscreen, Super+q request close, Super+1..9 switch, Super+Shift+1..9 move focused client, Super+Shift+Escape graceful quit/remap. Caps/Num locks are tolerated. There is no terminal launcher, full ICCCM/EWMH implementation, selection ownership announcement, decoration management, focus timestamps, WM_TAKE_FOCUS support or RandR integration. Pointer focus is off by default. X11 currently uses built-in policy defaults, not the CLI config option.

## History provenance

See [retrospective local history](history-provenance.md). Curated 2021–2022 timestamps are not original implementation dates.

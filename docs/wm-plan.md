# Portable WM core implementation plan

> **For Hermes:** implement small TDD slices; keep the installer and dotfiles intact.

**Goal:** add a C++17, dependency-free window management policy engine with a runnable command-driven harness.

**Architecture:** backend-independent geometry, layouts and workspace state; strict config and normalized keybindings; commands exercise the same state machine as a future compositor adapter.

**Tech stack:** C++17, CMake, CTest, no downloaded test framework.

## Ordered slices and gates
1. Create CMake/CTest harness, then geometry helpers (rectangle edges, inset, clipping, translation).
2. Implement master/stack, monocle, grid and bottom-stack layouts; test odd pixel conservation, empty inputs and gaps.
3. Implement window lifecycle, focus, floating/fullscreen, workspace movement, monitors and state snapshots. Each behavior gets a failing test before code.
4. Implement strict configuration and normalized keybindings in an isolated worktree; merge only after tests pass.
5. Connect config and commands to a deterministic executable; exercise it through CTest.
6. Document policy, backend boundaries and integration; run strict compiler and sanitizer builds.
7. Locally rewrite the requested chronology and GPG-sign using commit-filter; verify every commit, total and date distribution. Never push.

Every code slice runs the dependency-free focused tests RED then GREEN, then commits the behavior with an accurate message. Preserve upstream author identity, file history and an untouched archival branch.

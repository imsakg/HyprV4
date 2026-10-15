# Verification and local history operations

## Tested gates

```sh
cmake -S . -B build -G Ninja -DHYPRV4_BUILD_X11=ON
cmake --build build
ctest --test-dir build --output-on-failure
cmake -S . -B build-sanitize -G Ninja -DHYPRV4_SANITIZE=ON -DHYPRV4_BUILD_X11=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
DISPLAY= build/wm/hyprv4-x11 # expected diagnostic and exit 2; do not launch on a personal desktop
```

Observed on macOS 27 / AppleClang 21 / CMake / Xlib 1.8.13: normal and ASan+UBSan builds each pass **13 CTest cases** (11 C++ suites plus CLI version and scripted session). Strict `-Wall -Wextra -Wpedantic -Werror` compilation is enabled for executables. No runtime sanitizer findings were observed.

The property suite runs a layout-bounds matrix over tiny/zero/odd output dimensions for all four layouts and 5,000 fixed-seed client lifecycle transitions, checking invariants after each step. Other suites cover parser invalids/roundtrips, key aliases/collisions/lock events, negative coordinates, overflow, focus/fullscreen/workspace moves, floating stacking, quoted commands and key recursion limits.

An independent source review identified an X11 stacking-event loop risk, hidden-client shutdown bookkeeping and outer-border coordinate errors. Each received a regression test in the extracted backend-state helpers and a dedicated fix. A nested display server was unavailable: live X11 client integration, protocol compliance and display shutdown behavior still require Xvfb/Xephyr testing on Linux. Passing policy tests is not a claim of production compositor readiness.

## Requested retrospective history

The requested target is **80 new nonempty feature/test/build/documentation commits** in addition to **23 upstream commits**. Preserve `archive/hyprv4-upstream-2023` unchanged and preserve the complete pre-rewrite branch before rewriting main. Preserve every commit tree, subject, author name and author email across the chronology rewrite; only metadata/parent hashes/signatures change.

Use a sorted random sample of weekdays from March 2021 through February 2022 with independent evening times (18:00–23:59 at UTC+03). Save a manifest mapping original commit IDs to planned times outside the checkout. Apply `git rebase --root --force-rebase --exec ...`; each exec amends using explicit `git commit --amend --no-edit --date TIMESTAMP` and `GIT_COMMITTER_DATE=TIMESTAMP`. Do not change original author identities.

Finally sign the local main branch with `git filter-branch --commit-filter 'git commit-tree -S56611C482092733218A5D24BD860828DB5ED4BAC "$@"' -- main`. Signature creation timestamps must remain the real signing time: do not fake GPG's clock. The key was created after the curated range, so these signatures attest only to the present rewrite, never historical authorship.

Read back and verify all main commits with `git verify-commit`, compare original/rewritten ordered tree+subject+author manifests, and assert total count, nonempty new diffs, timestamp order, date range, weekdays and evening bounds. Check `git diff --check`, a clean worktree, and unchanged origin refs. Never push as part of this operation. See [provenance](history-provenance.md).

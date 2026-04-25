# DM1 all-graphics phase 137 — clean title formatter NUL literals

## Problem

The raw-title formatter contained literal embedded NUL characters in C char literals. Clang accepted it but warned during every build, which makes real regressions easier to miss.

## Change

Replaced embedded NUL char literals with explicit `\0` char constants in `m11_format_champion_title`.

No behavior change intended.

## Gate

```text
cmake --build build -j4: no "null character(s) preserved in char literal" warnings
firestaff_m11_game_view_probe: 445/445 invariants passed
ctest --test-dir build --output-on-failure: 5/5 PASS
```

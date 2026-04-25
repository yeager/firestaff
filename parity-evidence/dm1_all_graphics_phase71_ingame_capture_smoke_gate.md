# DM1 all-graphics phase 71 — CTest gate for deterministic in-game capture

## Problem

The capture pipeline already caused one false visual regression because `capture_ingame_series` was previously an ad-hoc/stale binary. Wiring it into CMake fixed the build target, but `ctest` still did not exercise the capture executable. That left room for future include/palette/build regressions to pass normal tests while screenshots silently rotted.

## Change

Added `run_firestaff_m11_ingame_capture_smoke.sh` and registered it as CTest:

```text
m11_ingame_capture_smoke
```

The smoke test:

- runs `${BUILD_DIR:-build}/capture_ingame_series`
- uses `${FIRESTAFF_DATA:-$HOME/.firestaff/data}`
- writes to a temporary directory
- verifies all six deterministic in-game `.ppm` captures exist
- verifies each output is binary PPM/P6 and nontrivial size

This is a pipeline guard, not a pixel-perfect art-parity assertion.

## Gate

```text
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 400/400 invariants passed

ctest --test-dir build --output-on-failure
1/5 Test #1: m11_phase_a ......................   Passed
2/5 Test #2: m11_game_view ....................   Passed
3/5 Test #3: m11_audio ........................   Passed
4/5 Test #4: m11_launcher_smoke ...............   Passed
5/5 Test #5: m11_ingame_capture_smoke .........   Passed
100% tests passed, 0 tests failed out of 5
```

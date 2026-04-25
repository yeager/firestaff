# DM1 all-graphics phase 70 — in-game capture matches runtime palette reset

## Problem

The real launcher path resets the renderer palette level when DM1 opens:

```c
main_loop_m11.c: M11_Render_SetPaletteLevel(0)
```

The deterministic in-game capture tool still kept the startup-menu palette level it read before opening DM1. If a nonzero startup palette setting is active, the capture can diverge from the real runtime and over-dim bare indexed pixels.

## Change

`verification-screens/capture_firestaff_ingame_series.c` now resets its local capture palette level to `0U` immediately after `M11_GameView_OpenSelectedMenuEntry(...)`, matching the real launcher handoff.

## Visual verification

Fresh capture:

- `verification-m11/capture-runtime-palette-reset-20260425-153139/01_ingame_start_latest.png`
- `verification-m11/capture-runtime-palette-reset-20260425-153139/05_ingame_after_cast_latest.png`

Image review:

- no rainbow/static palette corruption
- spell/menu overlay remains readable and presentable
- remaining texture speckle reads as dark DM-style dither rather than capture/runtime palette failure

## Gate

```text
cmake --build build --target capture_ingame_series -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 400/400 invariants passed
ctest --test-dir build --output-on-failure
100% tests passed, 0 tests failed out of 4
```

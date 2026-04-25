# DM1 all-graphics phase 78 — capture PPM geometry guard

## Problem

The in-game capture smoke test verified the existence of the six deterministic PPM screenshots and guarded against the stale palette path, but it still accepted any nontrivial P6 file. A malformed capture with the wrong logical size, max value, or truncated/extra payload could pass the smoke test and later mislead visual comparisons.

## Change

Strengthened `run_firestaff_m11_ingame_capture_smoke.sh` so every deterministic capture must be exactly:

```text
P6
320 200
255
payload bytes = 320 * 200 * 3
```

This locks the capture contract to Firestaff's 320×200 logical DM1 frame before the existing stale-palette viewport sanity check runs.

## Gate

```text
./run_firestaff_m11_ingame_capture_smoke.sh
In-game capture smoke PASS: 6 screenshots

./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 410/410 invariants passed

ctest --test-dir build --output-on-failure
5/5 PASS
```

# DM1 all-graphics phase 95 — capture-smoke gate for action vs inventory icon palettes

## Problem

Phases 93–94 made the screenshot fixture deterministic, but the `m11_ingame_capture_smoke` test still only verified that the six screenshots existed, had valid PPM geometry, and were not stale EGA-palette captures. It did not verify that the deterministic dagger fixture actually exercised the source object-icon palette distinction:

- action cell: source dagger icon with `G0498` remap (`12 -> C04 cyan`)
- inventory slot: same source dagger icon without that action-area remap

## Change

Extended `run_firestaff_m11_ingame_capture_smoke.sh` with pixel checks against the generated PPMs:

- `02_ingame_turn_right_latest.ppm`
  - counts DM cyan `(0,219,219)` in champion slot 0 action-icon inner box (`x=235..250`, `y=95..110`)
  - requires substantial coverage, proving the deterministic dagger hit the action-cell source-icon path with `G0498`
- `06_ingame_inventory_panel_latest.ppm`
  - counts source dark gray `(73,73,73)` in the right-hand inventory slot icon inset (`x=34..49`, `y=53..68`)
  - requires substantial coverage, proving inventory preserved source color 12 and did not inherit the action palette remap

## Gate

```text
./run_firestaff_m11_ingame_capture_smoke.sh
In-game capture smoke PASS: 6 screenshots
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 418/418 invariants passed
ctest --test-dir build --output-on-failure
5/5 PASS
```

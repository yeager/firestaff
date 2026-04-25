# DM1 all-graphics phase 8 — floor/ceiling base and viewport overdraw fix

Date: 2026-04-25 11:05 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 viewport base composition.

## Changes

### Stop using graphic 0 as dungeon background

Firestaff previously treated graphic `0000` (`224x136`) as a full dungeon viewport background. Comparison/inspection showed this was wrong: graphic 0 is a UI/panel-style graphic, not the DM1 dungeon base. This was a major reason the viewport looked like a random framed room/control panel.

Normal V1 now builds the viewport base from the source-backed floor/ceiling graphics used by ReDMCSB:

- `M650_GRAPHIC_FLOOR_SET_0_FLOOR = 78` (`224x97`)
- `M651_GRAPHIC_FLOOR_SET_0_CEILING = 79` (`224x39`)

Source anchors:

- `redmcsb-output/I34E_I34M/DEFS.H`
- `redmcsb-output/I34E_I34M/DUNVIEW.C F0094_DUNGEONVIEW_LoadFloorSet`
- `redmcsb-output/I34E_I34M/DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF`
- `C700_ZONE_VIEWPORT_CEILING_AREA`
- `C701_ZONE_VIEWPORT_FLOOR_AREA`

### Avoid normal-V1 viewport overdraw

The right status/action column and bottom area were still using old Firestaff rectangles that overlapped the source-faithful viewport. Normal V1 now aligns the right column to the viewport edge (`x=224`) and keeps the bottom fill below viewport bottom (`y=169`) instead of erasing parts of the `224x136` viewport.

Old Firestaff frame-strip asset drawing is now debug-only; in normal V1 it overdraws the floor/ceiling base and is not source-faithful.

## Artifacts

Generated normal V1 screenshot set:

- `verification-m11/dm1-all-graphics/phase8-floor-ceiling-base-20260425-1105/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase8-floor-ceiling-base-20260425-1105/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase8-floor-ceiling-base-20260425-1105/normal/party_hud_top_190_crop.png`

Visual inspection: the viewport is less wrong than the previous graphic-0/panel-background state and no longer uses the random UI/panel graphic as the dungeon base. It is still not 1:1: perspective assembly, wall draw order, masking, right panel, and bottom HUD remain unfinished.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Remaining work

Next viewport pass must port/bind the real `DUNVIEW.C` draw order:

- replace procedural corridor/wall drawing with source-bound wall face/side/door zones
- bind wall set graphics (`M656...`) to their original zones
- preserve floor/ceiling base and draw objects/creatures/projectiles over it in the original order
- add original-vs-Firestaff crop comparison for fixed map/x/y/facing/light

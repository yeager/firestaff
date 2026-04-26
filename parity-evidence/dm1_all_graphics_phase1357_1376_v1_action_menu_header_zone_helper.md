# DM1 all-graphics parity — phase 1357–1376: V1 action menu header zone helper

## Scope

Share the V1 action-menu champion-name header band geometry between rendering and probes, matching the source F0387 menu-mode header.

## Source anchors

- `ACTIDRAW.C F0387_MENUS_DrawActionArea` menu branch: the acting champion name is drawn in the cyan header band above the three trigger rows.
- Existing Firestaff V1 action area is the source `C011` right-column panel at `(224,45,87,45)`.
- Current source-backed header placement is `(224,47,87,9)` with black champion-name text on cyan.

## Implemented

- Added `M11_GameView_GetV1ActionMenuHeaderZone(...)` for probe-visible header geometry.
- Routed the action-menu cyan header fill through that helper.
- Added direct invariant coverage for the header rectangle.

## New invariant

- `INV_GV_300G`: action menu header zone matches F0387 source header geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `479/479 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300G action menu header zone matches F0387 source header geometry
# summary: 479/479 invariants passed
```

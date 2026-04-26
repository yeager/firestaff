# DM1 all-graphics parity — phase 1437–1456: V1 action menu text origin helper

## Scope

Share the V1 action-menu text origin geometry for the acting-champion header and three action rows, keeping text placement locked to the F0387 row/header zones.

## Source anchors

- `ACTIDRAW.C F0387_MENUS_DrawActionArea` menu branch prints the acting champion name in the header and action names in the trigger rows.
- Firestaff V1 action-menu text x-anchor is `226`, 2 px inside the source action area.
- Header text origin is `(226,48)`; row text origins are `(226,59)`, `(226,70)`, `(226,81)`.

## Implemented

- Added `M11_GameView_GetV1ActionMenuTextOrigin(...)` for probe-visible header/row text origins.
- Routed action-menu header text and row action-name drawing through the helper.
- Added invariant coverage for header, first row, and third row text origins.

## New invariant

- `INV_GV_300J`: action menu text origins match F0387 header/row offsets.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `483/483 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300J action menu text origins match F0387 header/row offsets
# summary: 483/483 invariants passed
```

# DM1 all-graphics parity — phase 1337–1356: V1 action menu row zone helper

## Scope

Share the V1 action-menu row geometry between rendering, pointer hit-testing, and probes so the three trigger rows no longer drift via duplicated constants.

## Source anchors

- `ACTIDRAW.C F0387_MENUS_DrawActionArea` menu branch: action menu rows are the three source trigger rows under the champion-name header.
- Existing Firestaff V1 action area is the source `C011` right-column area at `(224,45,87,45)`.
- The reconstructed F0387 row placement used by current parity is row 0 `(224,58,87,9)`, row 1 `(224,69,87,9)`, row 2 `(224,80,87,9)`.

## Implemented

- Added `M11_GameView_GetV1ActionMenuRowZone(...)` for probe-visible row geometry.
- Routed `M11_GameView_HandlePointer(...)` row hits through the shared helper.
- Routed `m11_draw_dm_action_menu(...)` row fills/text placement through the same helper.

## New invariant

- `INV_GV_300F`: action menu row zones match F0387 source row geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `478/478 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300F action menu row zones match F0387 source row geometry
# summary: 478/478 invariants passed
```

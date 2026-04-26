# DM1 all-graphics parity — phase 1237–1256: V1 action icon pointer zone parity

## Scope

Keep right-column action icon click geometry 1:1 with the source action-cell zones instead of a second handwritten formula.

## Source anchors

- `firestaff_pc34_core_amalgam.c:7866-7869`: mouse commands `C116..C119` bind to `C089..C092_ZONE_ACTION_AREA_CHAMPION_*_ACTION`.
- `firestaff_pc34_core_amalgam.c:11826-11863` (`F0386_MENUS_DrawActionIcon`) draws the same champion action icon cells.
- `zones_h_reconstruction.json` layout-696: `C092` resolves to the fourth champion cell `(299,86,20,35)`, so the inclusive visible bottom/right edge is `(318,120)`.

## Implemented

- Routed `M11_GameView_HandlePointer()` action-cell hit testing through `M11_GameView_GetV1ActionIconCellZone(...)`.
- Removed the separate pointer-side action-cell x/y/w/h formula so render, probe, and mouse hit geometry share the same source-backed zone helper.
- Added a rightmost-cell pointer invariant using a four-champion synthetic party.

## New invariant

- `INV_GV_300E`: clicking `(318,120)` activates champion ordinal 4 via source `C092` rightmost-cell geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `474/474 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300E action-hand icon pointer hit uses source C092 rightmost cell geometry
# summary: 474/474 invariants passed
```

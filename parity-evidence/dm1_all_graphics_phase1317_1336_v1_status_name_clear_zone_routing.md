# DM1 all-graphics parity — phase 1317–1336: V1 status name clear zone routing

## Scope

Remove the last local arithmetic from V1 compact champion-name clear drawing so both clear and text passes use the source-backed status-name zone helpers.

## Source anchors

- `zones_h_reconstruction.json` from GRAPHICS.DAT layout `C696`:
  - `C159..C162` are the compact status-name clear zones, `43×7`, at each status-box origin.
  - `C163..C166` are the text child zones used by the previous pass.
- `CHAMPION.C F0292_CHAMPION_DrawChampionState`: clears the status-name zone before drawing the champion name in source colors.

## Implemented

- Routed V1 champion-name clear fill through existing `M11_GameView_GetV1StatusNameZone(...)` instead of duplicating `x + M11_V1_STATUS_NAME_CLEAR_*` arithmetic at the draw site.
- Kept the existing source invariant `INV_GV_15E2` as coverage for the clear-zone geometry and `INV_GV_15E4` for rendered text/color inside that zone.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `477/477 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E2 V1 champion HUD name clear zones match layout-696 C159..C162 geometry
PASS INV_GV_15E4 V1 champion HUD renders source-colored names inside the compact status name zones
# summary: 477/477 invariants passed
```

# DM1 all-graphics parity — phase 1217–1236: V1 action icon zone helpers

## Scope

Harden right-column V1 action-hand icon cell parity by exposing and probing the source layout-696 geometry for the action cell and inner icon zones.

## Source anchors

- `zones_h_reconstruction.json` from GRAPHICS.DAT layout graphic `C696_GRAPHIC_LAYOUT`:
  - `C089..C092_ZONE_ACTION_AREA_CHAMPION_*_ACTION` resolve to four 20×35 cells at `(233,86)`, `(255,86)`, `(277,86)`, `(299,86)`.
  - `C093..C096_ZONE_ACTION_AREA_CHAMPION_*_ACTION_ICON` resolve to the same cell origins with the 16×16 icon bitmap clipped at `(cellX+2,95)`.
- `firestaff_pc34_core_amalgam.c:11826-11863` (`F0386_MENUS_DrawActionIcon`) draws each champion's action icon using `P0760_ui_ChampionIndex + C089` and blits the icon into `P0760_ui_ChampionIndex + C093`.
- `firestaff_pc34_core_amalgam.c:7866-7869` binds mouse commands to `C089..C092`, so hit/render geometry must stay in sync.

## Implemented

- Added probe-visible helpers:
  - `M11_GameView_GetV1ActionIconCellZone(...)`
  - `M11_GameView_GetV1ActionIconInnerZone(...)`
- Routed the V1 action icon renderer through the shared cell-zone helper instead of a local repeated formula.
- Added a direct invariant that verifies first and fourth source cell/inner zones against layout-696 coordinates, including the rightmost cell ending at x=318.

## New invariant

- `INV_GV_300D`: action-hand icon cell zones resolve to layout-696 `C089..C096` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `473/473 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300D action-hand icon cell zones resolve to layout-696 C089..C096 geometry
# summary: 473/473 invariants passed
```

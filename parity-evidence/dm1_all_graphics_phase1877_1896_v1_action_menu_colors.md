# DM1 all-graphics parity — phase 1877–1896: V1 action-menu colors

## Scope

Make the V1 action-menu header/row colors explicit and probe-visible instead of leaving F0387 palette constants inline in the renderer.

## Source anchors

- ReDMCSB `ACTIDRAW.C` / `F0387_MENUS_DrawActionArea` menu-mode branch fills the acting champion header band cyan and prints the champion name in black.
- Action rows are black strips with cyan action-name text.
- This pass only exposes and routes existing V1 colors; geometry and action dispatch are unchanged.

## Implemented

- Added action-menu color helpers:
  - `M11_GameView_GetV1ActionMenuHeaderFillColor()`
  - `M11_GameView_GetV1ActionMenuHeaderTextColor()`
  - `M11_GameView_GetV1ActionMenuRowFillColor()`
  - `M11_GameView_GetV1ActionMenuRowTextColor()`
- Routed `m11_draw_dm_action_menu(...)` header fill/text and row fill/text styles through the helpers.
- Added invariant coverage for the F0387 cyan/black color pairings.

## New invariant

- `INV_GV_300O`: action menu colors match F0387 cyan header/black name and black rows/cyan actions.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300O action menu colors match F0387 cyan header/black name and black rows/cyan actions
```

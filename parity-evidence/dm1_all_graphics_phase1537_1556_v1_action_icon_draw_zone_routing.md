# DM1 all-graphics parity — phase 1537–1556: V1 action icon draw zone routing

## Scope

Route the V1 action-hand icon-cell draw path through the shared source-backed action icon cell/inner-zone helpers instead of reusing the same F0386 literals at the draw site.

## Source anchors

- ReDMCSB `MENUS.C F0386_MENUS_DrawActionIcon`:
  - cell origin: `x = 233 + championIndex * 22`, `y = 86`
  - cell bounds: `20×35` (`X1..X2 = 20`, `Y1..Y2 = 35`)
  - icon blit box: `x + 2`, `y = 95`, `16×16`
- Existing probe-visible helpers:
  - `M11_GameView_GetV1ActionIconCellZone(...)`
  - `M11_GameView_GetV1ActionIconInnerZone(...)`
- Existing invariant `INV_GV_300D` verifies slot 0 and slot 3 cell/inner geometry.

## Implemented

- Routed dead-cell black fills through `M11_GameView_GetV1ActionIconCellZone(...)`.
- Routed living-cell cyan fills through `M11_GameView_GetV1ActionIconCellZone(...)`.
- Routed inner icon backdrop fills and object/empty-hand blits through `M11_GameView_GetV1ActionIconInnerZone(...)`.
- Routed lockout hatching through the resolved cell zone.
- No V2/startup-menu paths changed.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300D action-hand icon cell zones resolve to layout-696 C089..C096 geometry
```

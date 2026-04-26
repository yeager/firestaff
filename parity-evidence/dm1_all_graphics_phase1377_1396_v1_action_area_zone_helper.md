# DM1 all-graphics parity — phase 1377–1396: V1 action area zone helper

## Scope

Expose and reuse the source `C011_ZONE_ACTION_AREA` right-column rectangle across action-area blits, clears, menu mode, icon mode, and probes.

## Source anchors

- `DEFS.H C011_ZONE_ACTION_AREA`: DM1 right-column action area.
- GRAPHICS.DAT `C010_GRAPHIC_MENU_ACTION_AREA`: `87×45` action panel bitmap.
- Firestaff V1 placement for the source action panel is `(224,45,87,45)`.
- `ACTIDRAW.C F0387_MENUS_DrawActionArea`: both menu mode and icon mode begin by clearing/filling this action area before drawing content.

## Implemented

- Added `M11_GameView_GetV1ActionAreaZone(...)` for probe-visible source action-area geometry.
- Routed action-area menu clear/reblit through the helper.
- Routed utility-panel action frame blit and fallback clear through the helper.
- Routed icon-mode pre-cell black clear through the helper.

## New invariant

- `INV_GV_300H`: action area zone matches source `C011` right-column geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `480/480 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300H action area zone matches source C011 right-column geometry
# summary: 480/480 invariants passed
```

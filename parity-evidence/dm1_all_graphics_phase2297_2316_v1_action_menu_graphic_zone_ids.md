# DM1 all-graphics parity — phase 2297–2316: V1 action menu graphic zone ids

## Scope

Expose the source action-menu graphic zone selected from the number of visible action rows.

## Source anchors

`ACTIDRAW.C:F0387_MENUS_DrawActionArea` selects the action-area graphic zone by action-row count:

- one action row: `C079_ZONE_ACTION_AREA_ONE_ACTION_MENU`
- two action rows: `C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU`
- three action rows: `C011_ZONE_ACTION_AREA`

The source graphic remains `C010_GRAPHIC_MENU_ACTION_AREA` (`GRAPHICS.DAT` graphic id 10).

## Implemented

- Added `M11_GameView_GetV1ActionMenuGraphicZoneId()` returning `79`, `77`, or `11` for the F0387 action-row cases.
- Extended the right-column panel invariant so the action graphic id and its source menu-zone ids are verified together.

## Updated invariant

- `INV_GV_300P`: right-column V1 action graphic uses source `C010` with `C079/C077/C011` menu zones.

## Note

This pass exposes and verifies the source zone selection. It intentionally does not change the existing V1 draw crop yet; the current code still blits the full action-area graphic and draws blank rows for missing actions, as documented in the existing bounded comment near the F0387 menu implementation.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300P right-column V1 action graphic uses source C010 with C079/C077/C011 menu zones
```

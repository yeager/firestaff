# DM1 all-graphics parity — phase 1097–1196: V1 action icon area clear

## Scope

Tighten right-column action icon mode parity by restoring the source black clear of the action area before drawing champion action-hand cells.

## Source anchors

- `firestaff_pc34_core_amalgam.c:11876-11880` (`F0387_MENUS_DrawActionArea`): source always calls `F0733_FillZoneByIndex(C011_ZONE_ACTION_AREA, C00_COLOR_BLACK)` before the icon-mode branch draws each champion action icon.
- `firestaff_pc34_core_amalgam.c:11878-11879`: when `G0509_B_ActionAreaContainsIcons` is true, source draws only the action icon cells after that black clear.
- `dm7z-extract/Toolchains/Common/Source/DEFS.H:2216`: `C011_ZONE_ACTION_AREA` is the right-column action area.

## Implemented

- V1 icon mode now fills the source action-area rectangle (`224,45,87,45`) black immediately before drawing `F0386`-style action-hand cells.
- The spell-area frame below remains intact; the tall icon cells still overdraw y=86..120 as before.
- Menu mode already performed its own fill/reblit path, so this pass affects idle icon mode only.

## New invariant

- `INV_GV_300A`: action icon mode leaves the action-area top band overwhelmingly black before the cyan cells begin.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe -j2`
- `ctest --test-dir build --output-on-failure`
- `./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- Probe result: `471/471 invariants passed`

## Probe excerpt

```text
PASS INV_GV_300 action-hand icon cells: both living champions get cyan backdrop (or no assets)
PASS INV_GV_300A action icon mode fills the source action area top band black before drawing cells
PASS INV_GV_300B action-hand icon cells: empty living hand blits source empty-hand icon
PASS INV_GV_301 action-hand icon cells: dead champion cell is solid black
# summary: 471/471 invariants passed
```

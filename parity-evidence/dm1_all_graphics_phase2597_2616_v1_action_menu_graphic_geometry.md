# DM1 all-graphics parity — phase 2597–2616: V1 action menu graphic geometry

## Scope

Expose source-sized rectangles for the V1 action-menu background graphic zones selected by action row count.

## Source anchors

ReDMCSB `DEFS.H` / layout-696 reconstruction:

- `C079_ZONE_ACTION_AREA_ONE_ACTION_MENU` — one-action menu patch, `87×21`.
- `C077_ZONE_ACTION_AREA_TWO_ACTIONS_MENU` — two-action menu patch, `87×33`.
- `C011_ZONE_ACTION_AREA` — three-action/full action area, `87×45`.

## Implemented

- Added `M11_GameView_GetV1ActionMenuGraphicZone(actionRowCount, ...)` routed through `M11_GameView_GetV1ActionMenuGraphicZoneId(...)`.
- Preserved existing row-count id selection while exposing the source dimensions used for the selected menu graphic.

## Updated invariants

- `INV_GV_300AF`: asserts `C079/C077/C011` geometry for one-, two-, and three-row menu cases.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 517/517 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

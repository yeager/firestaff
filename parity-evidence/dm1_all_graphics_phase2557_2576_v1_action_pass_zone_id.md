# DM1 all-graphics parity — phase 2557–2576: V1 action PASS zone id

## Scope

Expose the layout-696 source zone id for the small right-aligned `PASS` label/hit area in the V1 action area.

## Source anchors

ReDMCSB `DEFS.H` / layout-696 reconstruction:

- `C011_ZONE_ACTION_AREA` — right-column action area at `(224,45,87,45)`.
- `C098_ZONE_ACTION_AREA_PASS` — `PASS` zone under the reconstructed `C097` 35×7 parent, right-aligned at the top of `C011`.
- `zones_h_reconstruction.json`: `C097` dimensions `35×7`; `C098` right-aligned within the action area (`d1=86,d2=0`).

## Implemented

- Added `M11_GameView_GetV1ActionPassZoneId()` returning source zone `C098`.
- Added `M11_GameView_GetV1ActionPassZone()` routed through the action-area helper.

## Updated invariants

- `INV_GV_300AD`: asserts `C098` id and `(275,45,35,7)` geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 515/515 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

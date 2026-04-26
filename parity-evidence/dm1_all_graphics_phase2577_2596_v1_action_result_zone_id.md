# DM1 all-graphics parity — phase 2577–2596: V1 action result zone id

## Scope

Expose the layout-696 source zone id used for action-result feedback in the right-column action area.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C011_ZONE_ACTION_AREA` — `(224,45,87,45)`.
- `C075_ZONE_ACTION_RESULT` — action-result zone under `C011`.

The reconstructed zone record for `C075` is a zero-offset child of `C011`, so its screen rectangle follows the action-area geometry.

## Implemented

- Added `M11_GameView_GetV1ActionResultZoneId()` returning source `C075`.
- Added `M11_GameView_GetV1ActionResultZone()` routed through `M11_GameView_GetV1ActionAreaZone()`.

## Updated invariants

- `INV_GV_300AE`: asserts `C075` id and `(224,45,87,45)` action-area geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 516/516 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

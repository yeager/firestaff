# DM1 all-graphics parity — phase 2697–2716: V1 viewport zone id

## Scope

Expose the layout-696 source zone id for the main V1 dungeon viewport and bind it to the existing DM1 PC 3.4 viewport rectangle.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C007_ZONE_VIEWPORT` — main dungeon viewport.
- Existing pass-40 Firestaff anchors: `M11_DM1_VIEWPORT_* = (0,33,224,136)`.

## Implemented

- Added `M11_GameView_GetV1ViewportZoneId()` returning source `C007`.
- Added `M11_GameView_GetV1ViewportZone()` exposing the existing DM1 viewport rectangle.

## Updated invariants

- `INV_GV_300AJ`: asserts `C007` id and `(0,33,224,136)` geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 522/522 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

# DM1 all-graphics parity — phase 2757–2776: V1 movement arrow geometry

## Scope

Expose resolved source rectangles for the V1 movement-arrow zones after the previous batch exposed their ids.

## Source anchors

`zones_h_reconstruction.json` / layout-696 resolves the six arrow controls under `C009_ZONE_MOVEMENT_ARROWS`:

- `C068` turn-left: `(234,125,19,21)`
- `C069` turn-right: `(291,125,19,21)`
- `C070` move-forward: `(263,125,27,21)`
- `C071` move-right: `(291,147,28,21)`
- `C072` move-backward: `(263,147,27,21)`
- `C073` move-left: `(234,147,28,21)`

## Implemented

- Added `M11_GameView_GetV1MovementArrowZone(index, ...)` for the six resolved arrow rectangles.
- Routed geometry through the source id helper for invalid-index rejection.

## Updated invariants

- Extended `INV_GV_300AL` to assert representative turn-left and move-right rectangles in addition to the source ids.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 523/523 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

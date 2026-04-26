# DM1 all-graphics parity — phase 2737–2756: V1 movement arrow zone ids

## Scope

Expose source zone-id mappings for the V1 movement-arrow control cluster.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C009_ZONE_MOVEMENT_ARROWS` — movement-arrow cluster root.
- `C068_ZONE_TURN_LEFT`
- `C069_ZONE_TURN_RIGHT`
- `C070_ZONE_MOVE_FORWARD`
- `C071_ZONE_MOVE_RIGHT`
- `C072_ZONE_MOVE_BACKWARD`
- `C073_ZONE_MOVE_LEFT`

## Implemented

- Added `M11_GameView_GetV1MovementArrowsZoneId()` returning root source `C009`.
- Added `M11_GameView_GetV1MovementArrowZoneId(index)` mapping the six source arrow zones and rejecting out-of-range indices.

## Updated invariants

- `INV_GV_300AL`: asserts `C009`, all six arrow ids `C068..C073`, and invalid-index rejection.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 523/523 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

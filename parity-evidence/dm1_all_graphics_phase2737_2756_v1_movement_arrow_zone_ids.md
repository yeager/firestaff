# DM1 all-graphics parity — phase 2737–2756: V1 movement arrow zone ids

## Scope

Expose source zone-id mappings and representative resolved rectangles for the V1 movement-arrow control cluster.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C009_ZONE_MOVEMENT_ARROWS` — movement-arrow cluster root.
- `C068_ZONE_TURN_LEFT`
- `C069_ZONE_TURN_RIGHT`
- `C070_ZONE_MOVE_FORWARD`
- `C071_ZONE_MOVE_RIGHT`
- `C072_ZONE_MOVE_BACKWARD`
- `C073_ZONE_MOVE_LEFT`

Representative resolved rectangles from `zones_h_reconstruction.json` via `tools/resolve_dm1_zone.py`:

- `C068` turn-left: `(234,125,19,21)`
- `C071` move-right: `(291,147,28,21)`

## Implemented

- Added `M11_GameView_GetV1MovementArrowsZoneId()` returning root source `C009`.
- Added `M11_GameView_GetV1MovementArrowZoneId(index)` mapping the six source arrow zones and rejecting out-of-range indices.
- Added `M11_GameView_GetV1MovementArrowZone(index, ...)` exposing resolved arrow rectangles.

## Updated invariants

- `INV_GV_300AL`: asserts `C009`, all six arrow ids `C068..C073`, invalid-index rejection, and representative arrow geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 523/523 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

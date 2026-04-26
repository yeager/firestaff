# DM1 all-graphics parity — phase 2657–2676: V1 leader hand object-name zone id

## Scope

Expose the layout-696 source zone id for the leader hand object-name readout in the V1 HUD chrome.

## Source anchors

ReDMCSB `DEFS.H` / layout-696 reconstruction:

- `C017_ZONE_LEADER_HAND_OBJECT_NAME` — leader hand object-name text zone.
- `zones_h_reconstruction.json`: parent text region `C016` dimensions `87×6`; `C017` offset `(233,33)`.

## Implemented

- Added `M11_GameView_GetV1LeaderHandObjectNameZoneId()` returning source `C017`.
- Added `M11_GameView_GetV1LeaderHandObjectNameZone()` exposing `(233,33,87,6)`.

## Updated invariants

- `INV_GV_300AH`: asserts `C017` id and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 520/520 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

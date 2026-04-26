# DM1 all-graphics parity — phase 2477–2496: V1 action area zone id

## Scope

Expose the layout-696 source zone id for the right-column action area and route action-area geometry validation through that id.

## Source anchors

Layout-696 / ReDMCSB `DEFS.H`:

- `C011_ZONE_ACTION_AREA`

The V1 action-area zone is the right-column action panel at `(224,45)` with size `87×45`.

## Implemented

- Added `M11_GameView_GetV1ActionAreaZoneId()` returning source zone `C011`.
- Routed `M11_GameView_GetV1ActionAreaZone()` through the zone-id helper.

## Updated invariants

- `INV_GV_300H`: action area zone exposes source `C011` id and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

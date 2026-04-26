# DM1 all-graphics parity — phase 2457–2476: V1 spell area zone id

## Scope

Expose the layout-696 source zone id for the right-column spell area and route spell-area geometry validation through that id.

## Source anchors

Layout-696 / ReDMCSB `DEFS.H`:

- `C013_ZONE_SPELL_AREA`

The V1 spell-area zone is the right-column spell panel at `(224,90)` with size `87×25`.

## Implemented

- Added `M11_GameView_GetV1SpellAreaZoneId()` returning source zone `C013`.
- Routed `M11_GameView_GetV1SpellAreaZone()` through the zone-id helper.

## Updated invariants

- `INV_GV_300I`: spell area zone exposes source `C013` id and geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```

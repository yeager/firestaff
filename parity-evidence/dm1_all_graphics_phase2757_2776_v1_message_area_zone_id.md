# DM1 all-graphics parity — phase 2757–2776: V1 message area zone id

## Scope

Expose the layout-696 source zone id and resolved rectangle for the bottom message area.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C015_ZONE_MESSAGE_AREA` — bottom message area.
- `zones_h_reconstruction.json`: `C015` is type `4`, parent `C014` (`320×27`), with bottom anchor `d2=199`.

Resolved with `tools/resolve_dm1_zone.py` for the full-width message area: `(0,176,320,24)`.

## Implemented

- Added `M11_GameView_GetV1MessageAreaZoneId()` returning source `C015`.
- Added `M11_GameView_GetV1MessageAreaZone()` exposing `(0,176,320,24)`.

## Updated invariants

- `INV_GV_300AM`: asserts `C015` id and full-width message-area geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — passed.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — `524/524 invariants passed`.
- `ctest --test-dir build --output-on-failure` — `5/5` tests passed.

# DM1 all-graphics parity — phase 2777–2796: V1 screen/dialog base zones

## Scope

Expose base layout-696 zone ids and resolved rectangles for the full screen and screen-centered dialog region.

## Source anchors

ReDMCSB `DEFS.H` / layout-696:

- `C002_ZONE_SCREEN` — full 320×200 screen root.
- `C005_ZONE_SCREEN_CENTERED_DIALOG` — dialog-sized region centered within the full screen.

`tools/resolve_dm1_zone.py` resolves `C005` with a `224×136` dialog bitmap to `(48,32,224,136)`.

## Implemented

- Added `M11_GameView_GetV1ScreenZoneId()` / `M11_GameView_GetV1ScreenZone()`.
- Added `M11_GameView_GetV1ScreenCenteredDialogZoneId()` / `M11_GameView_GetV1ScreenCenteredDialogZone()`.

## Updated invariants

- `INV_GV_300AN`: asserts `C002` full-screen geometry and `C005` centered-dialog geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — passed.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — `525/525 invariants passed`.
- `ctest --test-dir build --output-on-failure` — `5/5` tests passed.

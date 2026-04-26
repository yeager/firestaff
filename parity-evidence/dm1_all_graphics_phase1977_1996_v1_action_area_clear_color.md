# DM1 all-graphics parity — phase 1977–1996: V1 action-area clear color

## Scope

Make the V1 action-area pre-clear color explicit and use it consistently for action-menu and action-icon modes.

## Source anchors

- ReDMCSB `F0387_MENUS_DrawActionArea` clears the action area before drawing either action icons or the action menu.
- The clear is black; source graphics (`C010` action area, `C009` spell area background) then re-establish the right-column chrome.

## Implemented

- Added `M11_GameView_GetV1ActionAreaClearColor()` returning black.
- Routed action-menu mode pre-clear through the helper.
- Routed action-icon mode pre-clear through the helper.
- Kept panel graphics (`C010`/`C009`) unchanged.

## Updated invariant

- `INV_GV_300P`: right-column V1 panel graphics use source C010 action, black clear, and C009 spell-area background.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300P right-column V1 panel graphics use source C010 action, black clear, and C009 spell-area background
```

# DM1 all-graphics parity — phase 2257–2276: V1 dialog button zone ids

## Scope

Expose the source dialog button/hit zone id selected for each choice, then route pointer hit rectangles through that source id mapping.

## Source anchors

`DEFS.H` names the dialog pointer/button zones immediately before the visible choice text zones:

- `C456_ZONE_DIALOG_BOTTOM_BUTTON`
- `C457_ZONE_DIALOG_TOP_BUTTON`
- `C458_ZONE_DIALOG_TOP_LEFT_BUTTON`
- `C459_ZONE_DIALOG_TOP_RIGHT_BUTTON`
- `C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON`
- `C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON`

`DIALOG.C:F0427_DIALOG_Draw` pairs those button zones with the visible choice zones (`C462-C467`) for 1/2/3/4 choice layouts.

## Implemented

- Added `M11_GameView_GetV1DialogChoiceButtonZoneId()`.
- Routed `M11_GameView_GetV1DialogChoiceHitZone()` through the exposed source button-zone mapping.

## Updated invariant

- `INV_GV_300AB`: V1 dialog pointer hit zones expose source `C456-C461` button zones.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300AB V1 dialog pointer hit zones expose source C456-C461 button zones
```

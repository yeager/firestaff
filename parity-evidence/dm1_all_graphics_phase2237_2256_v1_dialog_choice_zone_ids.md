# DM1 all-graphics parity — phase 2237–2256: V1 dialog choice zone ids

## Scope

Expose the source dialog choice zone id selected for each visible choice, then route the existing choice text rectangles through that source id mapping.

## Source anchors

`DIALOG.C:F0427_DIALOG_Draw` prints choices through source zones:

- 1 choice: `C462_ZONE_DIALOG_BOTTOM_CHOICE`.
- 2 choices: `C463_ZONE_DIALOG_TOP_CHOICE`, `C462_ZONE_DIALOG_BOTTOM_CHOICE`.
- 3 choices: `C463_ZONE_DIALOG_TOP_CHOICE`, `C466_ZONE_DIALOG_BOTTOM_LEFT_CHOICE`, `C467_ZONE_DIALOG_BOTTOM_RIGHT_CHOICE`.
- 4 choices: `C464_ZONE_DIALOG_TOP_LEFT_CHOICE`, `C465_ZONE_DIALOG_TOP_RIGHT_CHOICE`, `C466_ZONE_DIALOG_BOTTOM_LEFT_CHOICE`, `C467_ZONE_DIALOG_BOTTOM_RIGHT_CHOICE`.

## Implemented

- Added `M11_GameView_GetV1DialogChoiceTextZoneId()`.
- Routed `M11_GameView_GetV1DialogChoiceTextZone()` through the exposed source zone id mapping.

## Updated invariant

- `INV_GV_300AA`: V1 dialog choice text zone ids expose source `C462-C467` layout cases.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300AA V1 dialog choice text zone ids expose source C462-C467 layout cases
```

# DM1 all-graphics parity — phase 1417–1436: V1 status box zone helper

## Scope

Expose the compact V1 champion status-box rectangles so the party HUD's base placement, child zones, and shield overlays share one source-backed status-box origin.

## Source anchors

- GRAPHICS.DAT `C007_GRAPHIC_STATUS_BOX`: `67×29` champion status box.
- `DEFS.H C69_CHAMPION_STATUS_BOX_SPACING`: source stride is `69` px.
- Firestaff V1 party HUD origin is `(12,160)`, so slot 0 is `(12,160,67,29)` and slot 3 is `(219,160,67,29)`.
- Layout-696 child zones already depend on this same status-box origin for names, bars, and hand slots.

## Implemented

- Added `M11_GameView_GetV1StatusBoxZone(...)` for probe-visible compact status-box geometry.
- Routed V1 party slot loop placement through the helper, leaving V2 HUD placement untouched.
- Added invariant coverage for first and fourth status-box rectangles.

## New invariant

- `INV_GV_15E9`: V1 champion HUD status box zones match `C007`/source stride geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `482/482 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E9 V1 champion HUD status box zones match C007/source stride geometry
# summary: 482/482 invariants passed
```

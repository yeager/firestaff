# DM1 all-graphics parity — phase 1257–1276: V1 status hand zone helper

## Scope

Harden compact champion status-box hand-slot placement by exposing the source layout-696 `C211..C218` geometry and routing drawing through the shared helper.

## Source anchors

- `zones_h_reconstruction.json` from GRAPHICS.DAT layout `C696`:
  - `C211/C213/C215/C217` ready-hand zones are parent status box + `(4,10)`, size `16×16`.
  - `C212/C214/C216/C218` action-hand zones are parent status box + `(24,10)`, size `16×16`.
- `firestaff_pc34_core_amalgam.c` / `CHAMDRAW.C F0291_CHAMPION_DrawSlot` draws ready/action hand slots into those status-box child zones.
- Existing V1 status-box stride is `69` pixels, so slot 0 ready zone is `(16,170,16,16)` and slot 3 action zone is `(243,170,16,16)` in Firestaff's screen placement.

## Implemented

- Added `M11_GameView_GetV1StatusHandZone(...)` for probe-visible, source-backed compact HUD hand-slot geometry.
- Routed V1 status-hand rendering through that helper instead of local `x + offset` placement.
- Added a layout invariant covering the first ready-hand and fourth action-hand zones.

## New invariant

- `INV_GV_15E6`: V1 status hand slot zones match layout-696 `C211..C218` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `475/475 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E6 V1 status hand slot zones match layout-696 C211..C218 geometry
# summary: 475/475 invariants passed
```

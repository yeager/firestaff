# DM1 all-graphics parity — phase 2117–2136: V1 dialog graphic zones

## Scope

Expose and route the source dialog backdrop, version text origin, and choice-patch zones used by the V1 dialog overlay.

## Source anchors

- `C000_GRAPHIC_DIALOG_BOX` is the 224×136 source dialog backdrop (GRAPHICS.DAT graphic 17 in the M11 asset index).
- `C450_ZONE_DIALOG_VERSION` prints `V3.4` at viewport-relative `(192, 7)` → framebuffer `(192, 40)` with the DM1 viewport origin `(0, 33)`.
- `M621/M622/M623` dialog patches are copied from source dialog-box regions for 1-, 2-, and 4-choice layouts; the 3-choice layout uses the unpatched backdrop.

## Implemented

- Added `M11_GameView_GetV1DialogBackdropGraphicId()` and routed dialog backdrop/patch asset loads through it.
- Added `M11_GameView_GetV1DialogVersionTextOrigin()` and routed version text draw through it.
- Added `M11_GameView_GetV1DialogChoicePatchZone()` and routed the dialog patch copier through it.

## Updated invariant

- `INV_GV_300X`: V1 dialog backdrop/version/choice patches use source `C000/C450/M621-M623` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 509/509 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300X V1 dialog backdrop/version/choice patches use source C000/C450/M621-M623 geometry
```

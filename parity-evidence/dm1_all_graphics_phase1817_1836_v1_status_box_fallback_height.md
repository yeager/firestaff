# DM1 all-graphics parity — phase 1817–1836: V1 status-box fallback height

## Scope

Make the V1 procedural status-box fallback preserve the source `67×29` champion status-box extent when GRAPHICS.DAT status-box assets are unavailable.

## Source anchors

- GRAPHICS.DAT `C007_GRAPHIC_STATUS_BOX` and `C008_GRAPHIC_STATUS_BOX_DEAD` are `67×29`.
- The V1 status-box helper already exposes `M11_GameView_GetV1StatusBoxZone(...) == 67×29`.
- V2 vertical-slice HUD remains legacy `71×28` and must not be changed by this pass.

## Implemented

- Updated the fallback branch in `m11_draw_party_panel(...)` to use `slotH` for V1 instead of hard-coded `M11_PARTY_SLOT_H` (`28`).
- Kept V2 fallback at `M11_PARTY_SLOT_H` to preserve the modern vertical-slice HUD alignment.
- Added a no-assets/dead-champion draw invariant to check the bottom row of the V1 `67×29` fallback box.

## New invariant

- `INV_GV_15Y`: V1 dead status-box fallback preserves source `67×29` `C008` extent.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15Y V1 dead status-box fallback preserves source 67x29 C008 extent
```

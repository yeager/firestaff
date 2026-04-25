# DM1 all-graphics parity — phase 49: object source scale/shift applied

Date: 2026-04-25 20:08 Europe/Stockholm

## Goal

Start using the source object placement support tables in the temporary center/side contents renderer, instead of merely pinning them for probes.

This is still not the final C2500 zone renderer, but it moves visible object scale/scatter behavior onto DM1 source data while the exact zone pass is prepared.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G2030_auc_ObjectScales[5] = {27, 21, 18, 14, 12}`
  - `G0217_aauc_Graphic558_ObjectPileShiftSetIndices[16][2]`
  - `G0223_aac_Graphic558_ShiftSets[3][8]`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - source scale-index formula: `(viewDepth * 2) - 1 - (viewCell >> 1)` after the native D1 case

## Implemented

- `m11_game_view.c`
  - Added per-item relative cell capture in `M11_ViewportCell.floorItemCells[]` from the high bits of the thing id (`thing >> 14`), rotated by party direction.
  - Added `m11_object_source_scale_index(depthIndex, relativeCell)` matching the F0115 front/back row scale bucket selection for D1/D2/D3 center-lane cells.
  - Updated `m11_draw_item_sprite(...)` to accept relative cell + pile index.
  - Replaced invented item depth percentage/scatter with:
    - source `G2030` scale units out of 32,
    - source `G0217` pile shift pair lookup,
    - source `G0223` shift value lookup.
  - Updated front and side contents calls to pass relative cell and pile index.

- `m11_game_view.h`
  - Exposed `M11_GameView_GetObjectSourceScaleIndex(...)` for probes.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114C2` verifying D1/D2/D3 front/back source scale-index selection:
    - D1 native bucket `0`
    - D2 front `1`, D2 back `2`
    - D3 front `3`, D3 back `4`

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114C2 object source scale-index selection follows F0115 front/back cells`
- `PASS INV_GV_38N focused viewport: D1C dagger object sprite changes the corridor frame`
- `PASS INV_GV_38O focused viewport: D1C object sprite with G0209 native-index gap changes the corridor frame`
- Probe summary: `385/385 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scenes:

- `verification-m11/dm1-all-graphics/phase49-object-source-scale-shift-20260425-2008/normal/37_focused_d1c_dagger_object_vga.png`
- `verification-m11/dm1-all-graphics/phase49-object-source-scale-shift-20260425-2008/normal/38_focused_d1c_object_native_gap_vga.png`

Visual inspection result:

- Objects visible in both focused captures.
- No obvious sprite corruption.
- No visible mask rectangles/bounding boxes.
- No significant clipping.
- No UI bleed.
- Objects are small but readable and placed on the floor plane, acceptable for this temporary renderer step.

## Remaining work

- Replace the temporary center/side item rectangle placement with exact source `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` zone blits.
- Add multi-object focused gates that prove pile shift progression/wrap behavior visually.
- Add alcove image and flip-on-right (`MASK0x0010_ALCOVE`, `MASK0x0001_FLIP_ON_RIGHT`) gates.

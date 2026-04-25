# DM1 all-graphics parity — phase 42: focused object gate

Date: 2026-04-25 17:25 Europe/Stockholm

## Goal

Add a focused visual regression gate for a visible D1C floor object. The earlier M612 pass corrected object graphics to `M612_GRAPHIC_FIRST_OBJECT = 498`, but did not yet have an isolated screenshot gate proving the normal viewport path actually draws a source-backed floor object.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `M612_GRAPHIC_FIRST_OBJECT = 498`
  - `C10_COLOR_FLESH`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0209_as_Graphic558_ObjectAspects`
  - `G0237_as_Graphic559_ObjectInfo`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - floor object path draws via `F0791_DUNGEONVIEW_DrawBitmapXX(..., C10_COLOR_FLESH)`

## Implemented

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added a focused D1C dagger/floor-object scene in the isolated corridor capture set.
  - Captures `37_focused_d1c_dagger_object_vga` when `PROBE_SCREENSHOT_DIR` is set.
  - Added `INV_GV_38N`: focused D1C dagger object changes the empty corridor frame.

This reuses the normal V1 center-lane contents pass added in phase 40, so object, creature, and projectile families now all have focused visibility gates.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_38N focused viewport: D1C dagger object sprite changes the corridor frame`
- Probe summary: `376/376 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase42-focused-object-gate-20260425-1725/normal/37_focused_d1c_dagger_object_vga.png`
- `verification-m11/dm1-all-graphics/phase42-focused-object-gate-20260425-1725/normal/37_focused_d1c_dagger_object_vga_viewport_only.png`

Visual inspection result:

- Dagger/floor object visible near the lower center of the scene.
- No obvious sprite corruption.
- Palette/colors look coherent.
- No visible mask rectangle around the object.
- No UI bleed.

## Remaining work

- Replace current approximate item placement/scatter with source zone placement:
  - `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`
  - `G0217_aauc_Graphic558_ObjectPileShiftSetIndices`
  - `G0223_aac_Graphic558_ShiftSets`
  - `G2030_auc_ObjectScales`
- Add focused side-lane and multi-object gates.
- Verify alcove object image selection (`MASK0x0010_ALCOVE`) and flip-on-right (`MASK0x0001_FLIP_ON_RIGHT`).

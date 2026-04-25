# DM1 all-graphics parity — phase 44: object G0209 native-index gaps

Date: 2026-04-25 18:10 Europe/Stockholm

## Goal

Fix remaining object viewport sprite index drift caused by synthesizing `G0209_as_Graphic558_ObjectAspects[].FirstNativeBitmapRelativeIndex` as `aspectIndex + 1`.

That shortcut works for many early aspects but fails at source gaps introduced by alternate/alcove/right-facing native images. Those gaps shift later object graphics by one or more indices.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `M612_GRAPHIC_FIRST_OBJECT = 498`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0209_as_Graphic558_ObjectAspects`
  - `G0237_as_Graphic559_ObjectInfo`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

Relevant source fact:

- `G0209[65].FirstNativeBitmapRelativeIndex = 67`
- therefore object aspect 65 uses graphic `498 + 67 = 565`, not stale `498 + (65 + 1) = 564`.

## Implemented

- `m11_game_view.c`
  - Added `kObjectAspectFirstNative[85]` copied from source `G0209_as_Graphic558_ObjectAspects[].FirstNativeBitmapRelativeIndex`.
  - Changed `m11_item_sprite_index(...)` to use the source table instead of `aspectIndex == 0 ? 0 : aspectIndex + 1`.
  - This fixes all G0209 native gaps, including aspects 65, 80, 81, 82, 83, and 84.

- `m11_game_view.h`
  - Added `M11_GameView_GetObjectSpriteIndex(...)` probe helper.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114B`:
    - weapon subtype 43 -> object-info index 66 -> aspect 65 -> source native relative 67 -> graphic 565.
  - Added focused `INV_GV_38O` and screenshot capture for a D1C floor object using this G0209 native-index gap.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114B object sprite uses G0209 firstNative gap: weapon subtype 43 -> aspect 65 -> graphic 565`
- `PASS INV_GV_38O focused viewport: D1C object sprite with G0209 native-index gap changes the corridor frame`
- Probe summary: `379/379 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase44-object-g0209-native-gaps-20260425-1810/normal/38_focused_d1c_object_native_gap_vga.png`
- `verification-m11/dm1-all-graphics/phase44-object-g0209-native-gaps-20260425-1810/normal/38_focused_d1c_object_native_gap_vga_viewport_only.png`

Visual inspection result:

- Object visible on D1C floor.
- No obvious sprite corruption.
- Palette looks coherent.
- No mask rectangle.
- No UI bleed.

## Remaining work

- Exact source zone placement/scatter for objects:
  - `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`
  - `G0217_aauc_Graphic558_ObjectPileShiftSetIndices`
  - `G0223_aac_Graphic558_ShiftSets`
  - `G2030_auc_ObjectScales`
- Alcove image/flip cases:
  - `MASK0x0010_ALCOVE`
  - `MASK0x0001_FLIP_ON_RIGHT`
- Focused side-lane and multi-object gates.

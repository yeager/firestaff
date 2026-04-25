# DM1 all-graphics parity — phase 52: object flip-on-right applied

Date: 2026-04-25 20:38 Europe/Stockholm

## Goal

Start applying DM1 object `MASK0x0001_FLIP_ON_RIGHT` behavior in the temporary object renderer.

This is still not the final C2500 zone renderer, but object sprites that request right-side flipping now follow the source flag and relative cell.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `MASK0x0001_FLIP_ON_RIGHT`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0209_as_Graphic558_ObjectAspects[].GraphicInfo`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - object path flips when the object's aspect requests it and the object is on the right lane / right-side center cell, excluding alcove image cases.

## Implemented

- `m11_game_view.c`
  - Added `m11_item_aspect_index(...)` to resolve thing type/subtype through the source `G0237` object-info aspect mapping.
  - Updated `m11_draw_item_sprite(...)` to mirror object sprites when:
    - `G0209[aspect].GraphicInfo & MASK0x0001_FLIP_ON_RIGHT`, and
    - relative cell is right-side (`1` or `3`).
  - Uses `M11_AssetLoader_BlitScaledMirror(..., transparentColor=10)` for mirrored object composition.

- `m11_game_view.h`
  - Exposed `M11_GameView_ObjectUsesFlipOnRight(...)` for probes.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114E3`:
    - container aspect 0 flips on right cell,
    - same object does not flip on left/back cell,
    - normal dagger does not flip on right cell.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114E3 object flip-on-right follows G0209 GraphicInfo and relative cell`
- Probe summary: `388/388 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase52-object-flip-on-right-20260425-2038/normal/39_focused_d1c_multi_object_shift_vga.png`

Visual inspection result:

- No obvious object sprite corruption.
- No mask rectangles.
- No visible clipping/bleed.
- UI remains contained.
- Tiny object scale makes subtle flip-origin issues hard to prove from this still, but no glaring regression was visible.

## Remaining work

- Add focused visual gate using a clearly asymmetric flip-on-right object once exact object selection/scene setup is convenient.
- Implement `MASK0x0010_ALCOVE` and C2548 alcove object path.
- Replace temporary object placement with exact `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` source zones.

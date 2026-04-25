# DM1 all-graphics parity — phase 50: focused multi-object pile gate

Date: 2026-04-25 20:20 Europe/Stockholm

## Goal

Add a focused visual regression gate for multiple floor objects in one D1C square, now that object drawing uses source `G0217`/`G0223` pile-shift data.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0217_aauc_Graphic558_ObjectPileShiftSetIndices[16][2]`
  - `G0223_aac_Graphic558_ShiftSets[3][8]`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `M011_CELL(thing) ((thing) >> 14)`

## Implemented

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Expanded the focused object scene harness to allocate two weapon things.
  - Built a deterministic two-item chain on D1C with different thing-cell bits:
    - first object in cell 0,
    - second object in cell 3.
  - Captures `39_focused_d1c_multi_object_shift_vga` when `PROBE_SCREENSHOT_DIR` is set.
  - Added `INV_GV_38P`: multi-object pile frame differs from the single-object frame.

The renderer changes from phase 49 already apply source scale/shift tables, so this pass primarily adds a visual gate around pile progression and cell-bit handling.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_38P focused viewport: D1C multi-object pile differs from single-object frame`
- Probe summary: `386/386 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase50-multi-object-pile-gate-20260425-2020/normal/39_focused_d1c_multi_object_shift_vga.png`
- `verification-m11/dm1-all-graphics/phase50-multi-object-pile-gate-20260425-2020/normal/39_focused_d1c_multi_object_shift_vga_viewport_only.png`

Visual inspection result:

- Multiple objects/pile visible: small yellow/black object on left and red/white object/pile on right.
- No obvious sprite corruption.
- No visible mask rectangles.
- No object clipping apparent.
- No UI bleed.

## Remaining work

- Replace temporary rectangle placement with exact `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` zone blits.
- Add alcove/flip-on-right focused gates.
- Later remove the temporary center-lane contents pass once the exact source zone object path covers all needed cells.

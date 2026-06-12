# pass787 CSB V1 D1C F0108 Floor/Ceiling/Ornament Source Lock

Contract-only gate for the CSB V1 D1C F0108 floor+ceiling+ornament path. It does not claim original-DOS pixel parity and does not load game data.

## Anchors

- ReDMCSB `DUNVIEW.C` `F0108_DUNGEONVIEW_DrawFloorOrnament`: lines 3940-4011.
- ReDMCSB `DUNVIEW.C` `F0124_DUNGEONVIEW_DrawSquareD1C`: lines 7873-7957, including the door-front F0108 branch at 7874, the corridor/pit/teleporter F0108 branch at 7926, `F0112` before `F0115`, and `F0113` after `F0115`.
- ReDMCSB `DUNVIEW.C` `F0128_DUNGEONVIEW_Draw_CPSF`: lines 8491-8499 for the preceding D3 center ordering and 8524-8542 for the D1C dispatch neighborhood.
- ReDMCSB `DUNVIEW.C` `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`: lines 4547-4581, 4923, 5180-5188, 5211-5214, 5458-5570, and 5668-5671.
- ReDMCSB `DUNGEON.C`: `F0163` lines 1769-1838, `F0164` lines 1840-1905, and `F0172` lines 2466-2523.
- ReDMCSB `DEFS.H`: `C10_COLOR_FLESH`, `M575..M579`, `M595`, `M606`, `C705/C706`, and `C1500`.
- CSB lineage `Viewport.cpp`: CustomBackgrounds masked application at 6507-6548 and the first CSB-only backdrop neighborhood at 6924-6927.

## Coverage

- Pins CSB V1 D1C as a CSB-specific lane, not the DM1 D1C body.
- Pins the 320x200 framebuffer and 224x136 viewport contract.
- Pins D1C floor zone `1505`, ReDMCSB PC34 floor-ornament zone `1509`, and CustomBackgrounds D1C slot ordinal `11`.
- Pins `C10_COLOR_FLESH` transparency for F0108 and thing-pass blits.
- Pins `MASK0x8000_FOOTPRINTS` recursion and the `C15_FLOOR_ORNAMENT_FOOTPRINTS` path.
- Pins D1C corridor/open-pit/teleporter one-pass ordering and door-front two-pass ordering (`0x0218` then `0x0349`).
- Explicitly keeps out the sibling F0115-only, F0107 wall-ornament, CustomBackgrounds room-slot/mask, and DM1 V1 slices.

## Verification

Focused local verification:
```text
PASS test_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_pc34_compat assertions=336 failures=0 custom_bg_masks=4 d1c_floor=1505 thing_passes=5 palette_keepouts=4 mutation_rejections=8 hash=0x4f3ba518
```

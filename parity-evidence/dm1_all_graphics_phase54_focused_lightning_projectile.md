# DM1 all-graphics parity — phase 54: focused lightning projectile gate

Date: 2026-04-25 21:10 Europe/Stockholm

## Goal

Add focused screenshot coverage for the projectile `G0210` bitmap-delta path introduced in phase 53.

Fireball is aspect type C3 and never rotates; lightning is aspect type C2 and should select `firstNative + 1` for right/left relative travel. This pass proves the lightning path appears visibly in the normal focused viewport harness.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0210_as_Graphic558_ProjectileAspects`
  - lightning aspect 3: `FirstNativeBitmapRelativeIndex = 9`, `GraphicInfo = 0x0112`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `C2_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_ROTATION`
  - `M613_GRAPHIC_FIRST_PROJECTILE = 454`

## Implemented

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added a focused D1C runtime lightning projectile scene.
  - The synthetic projectile uses `PROJECTILE_SUBTYPE_LIGHTNING_BOLT` and direction `DIR_EAST` while party faces north, producing relative direction `1` and therefore the C2 rotation delta `+1`.
  - Captures `40_focused_d1c_lightning_projectile_vga` when `PROBE_SCREENSHOT_DIR` is set.
  - Added `INV_GV_38Q`: lightning projectile frame differs from both empty corridor and fireball projectile frames.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_38Q focused viewport: D1C lightning projectile differs from empty and fireball frames`
- `PASS INV_GV_245D projectile G0210 aspect bitmap delta handles lightning rotation and fireball no-rotation`
- Probe summary: `390/390 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase54-focused-lightning-projectile-20260425-2110/normal/40_focused_d1c_lightning_projectile_vga.png`
- `verification-m11/dm1-all-graphics/phase54-focused-lightning-projectile-20260425-2110/normal/40_focused_d1c_lightning_projectile_vga_viewport_only.png`

Visual inspection result:

- Lightning visible as a small jagged yellow bolt near the lower-middle of the scene.
- No obvious sprite corruption.
- No visible mask rectangle/bounding box.
- No apparent clipping.
- No UI bleed.

## Remaining work

- Implement exact C0 projectile back-graphic + horizontal/vertical flip behavior from F0115.
- Bind projectile placement to the exact `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` zone path.
- Add side/subcell projectile gates after exact zone placement lands.

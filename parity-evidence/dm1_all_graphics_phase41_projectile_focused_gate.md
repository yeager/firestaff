# DM1 all-graphics parity — phase 41: focused projectile gate + C10 transparency

Date: 2026-04-25 17:10 Europe/Stockholm

## Goal

Add a focused visual regression gate for a visible D1C projectile and correct projectile transparency against DM1 source behavior.

The earlier M613 pass corrected projectile graphics to `M613_GRAPHIC_FIRST_PROJECTILE = 454`, but only had summary/runtime invariants. This pass adds a deterministic screenshot gate so projectile sprite rendering cannot silently disappear from the normal viewport path.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `M613_GRAPHIC_FIRST_PROJECTILE = 454`
  - `C10_COLOR_FLESH`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0210_as_Graphic558_ProjectileAspects`
  - `G0215_auc_Graphic558_ProjectileScales`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - projectile path jumps into `T0115015_DrawProjectileAsObject`
  - `F0791_DUNGEONVIEW_DrawBitmapXX(..., C10_COLOR_FLESH)`

## Implemented

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added focused runtime fireball at D1C `(2,2)` in the isolated corridor scene.
  - Captures `36_focused_d1c_fireball_projectile_vga` when `PROBE_SCREENSHOT_DIR` is set.
  - Added `INV_GV_38M`: focused D1C fireball projectile changes the corridor frame.

- `m11_game_view.c`
  - Changed projectile sprite blit transparency from palette `0` to palette `10` (`C10_COLOR_FLESH`, represented locally as `M11_COLOR_MAGENTA`).
  - Reason: DM1 draws projectiles through the object/projectile shared path and calls `F0791_DUNGEONVIEW_DrawBitmapXX` with `C10_COLOR_FLESH` as transparent key. Fireball native graphics have palette index 10 in their border; black transparency left a visible square backing.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Updated `INV_GV_245` to assert projectile transparency key `10` instead of stale key `0`.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_38M focused viewport: D1C fireball projectile sprite changes the corridor frame`
- `PASS INV_GV_245 projectile sprite transparency key is palette index 10 (C10_COLOR_FLESH)`
- Probe summary: `375/375 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase41-focused-projectile-c10-transparency-20260425-1710/normal/36_focused_d1c_fireball_projectile_vga.png`
- `verification-m11/dm1-all-graphics/phase41-focused-projectile-c10-transparency-20260425-1710/normal/36_focused_d1c_fireball_projectile_vga_viewport_only.png`

Visual inspection result:

- Fireball projectile visible.
- Palette appears coherent with red/yellow/orange fireball colors.
- No square backing/mask rectangle remains after switching transparency to palette 10.
- No obvious sprite corruption.
- No UI bleed.

## Remaining work

- Replace the current approximate projectile placement/scaling with the exact source zone path:
  - `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`
  - view depth / view cell scale index from `G0215_auc_Graphic558_ProjectileScales`
  - projectile aspect rotation/back-graphic/vertical-flip cases from `G0210.GraphicInfo`
- Add focused side-cell and sub-cell projectile visual gates.

# DM1 all-graphics parity — phase 43: projectile source scales

Date: 2026-04-25 17:40 Europe/Stockholm

## Goal

Move projectile sprite scaling away from the previous invented depth percentages and bind it to DM1's source scale table.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0215_auc_Graphic558_ProjectileScales[7] = {32, 27, 21, 18, 14, 12, 9}`
  - comments identify:
    - `32` D1 back/native
    - `27` D2 front
    - `21` D2 back
    - `18` D3 front
    - `14` D3 back
    - `12` D4 front
    - `9` D4 back
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - `AL0150_ui_ProjectileScaleIndex = (viewDepth << 1) - (viewCell >> 1)`

## Implemented

- `m11_game_view.c`
  - Added `m11_projectile_source_scale_units(depthIndex, relativeCell)`.
  - Replaced approximate projectile scale percentages (`100`, `66`, `40`) with source units out of 32.
  - D1 remains native scale (`32/32`).
  - D2/D3 now distinguish front-row vs back-row projectile sub-cells:
    - D2 front `27/32`, D2 back `21/32`
    - D3 front `18/32`, D3 back `14/32`
  - D4 values are clamped/available even though current normal V1 focused pass only draws through D3.

- `m11_game_view.h`
  - Exposed `M11_GameView_GetProjectileSourceScaleUnits(...)` for probes.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_245B` to pin the D1/D2/D3 front/back scale values to `G0215`.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_245B projectile source scale units match G0215 for D1/D2/D3 front/back cells`
- `PASS INV_GV_38M focused viewport: D1C fireball projectile sprite changes the corridor frame`
- Probe summary: `377/377 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Visual evidence

Captured focused scene:

- `verification-m11/dm1-all-graphics/phase43-projectile-source-scale-20260425-1740/normal/36_focused_d1c_fireball_projectile_vga.png`

Visual inspection result:

- Projectile/fireball remains coherent and correctly masked.
- No visible rectangular mask/boxed background.
- No UI bleed.
- No obvious scaling corruption.

## Remaining work

- Bind exact projectile zone placement to `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` instead of approximate quadrant offsets.
- Implement projectile aspect rotation/back-graphic/vertical-flip from `G0210.GraphicInfo`.
- Add focused D2/D3 and side-cell projectile screenshot gates so the new scale paths have direct visual coverage, not just helper invariants.

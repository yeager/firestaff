# DM1 all-graphics parity — phase 53: projectile G0210 bitmap delta applied

Date: 2026-04-25 20:52 Europe/Stockholm

## Goal

Start applying source `G0210_as_Graphic558_ProjectileAspects[].GraphicInfo` to projectile native bitmap selection, so rotating/no-back projectile aspects no longer always draw their first native bitmap.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0210_as_Graphic558_ProjectileAspects`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
- `../redmcsb-output/I34E_I34M/DEFS.H`
  - `MASK0x0003_ASPECT_TYPE`
  - aspect types:
    - `C0_PROJECTILE_ASPECT_TYPE_HAS_BACK_GRAPHIC_AND_ROTATION`
    - `C1_PROJECTILE_ASPECT_TYPE_HAS_BACK_GRAPHIC_AND_NO_ROTATION`
    - `C2_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_ROTATION`
    - `C3_PROJECTILE_ASPECT_TYPE_NO_BACK_GRAPHIC_AND_NO_ROTATION`

## Implemented

- `m11_game_view.c`
  - Added `firstProjectileSubtype` to `M11_ViewportCell` so runtime projectiles can be re-resolved after relative direction is known.
  - Added `m11_projectile_subtype_to_aspect_index(...)`.
  - Added `m11_projectile_aspect_bitmap_delta(...)` and `m11_projectile_aspect_to_graphic_index(...)`.
  - Runtime projectile sampling now updates `firstProjectileGfxIndex` after computing `firstProjectileRelDir`.
  - Covered source behavior currently includes:
    - aspect type C3: no rotation, always delta 0 (fireball/poison/default-like explosion projectile aspects).
    - aspect type C2: right/left relative travel uses delta +1, front/back uses delta 0 (lightning-style no-back rotating aspect).
    - bounded C0/C1 approximations are prepared but full vertical flip/back-graphic fidelity remains upcoming.

- `m11_game_view.h`
  - Exposed probe helpers:
    - `M11_GameView_GetProjectileAspectBitmapDelta(...)`
    - `M11_GameView_GetProjectileGraphicForAspect(...)`

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_245D`:
    - lightning aspect 3, relative right -> delta +1 -> graphic 464 (`454 + 9 + 1`).
    - lightning front/back -> delta 0.
    - fireball aspect 10 never rotates -> graphic 482 for relative right/left.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_245D projectile G0210 aspect bitmap delta handles lightning rotation and fireball no-rotation`
- Probe summary: `389/389 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Add focused lightning projectile visual gate to prove the delta path in screenshots.
- Implement exact C0 back-graphic + horizontal/vertical flip behavior from F0115.
- Bind projectile object-zone placement to `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`.

# DM1 all-graphics parity — phase 48: projectile G0210 aspect table pinned

Date: 2026-04-25 19:10 Europe/Stockholm

## Goal

Pin the DM1 projectile aspect table so the upcoming projectile rotation/back-graphic/flip pass has source-backed data available.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0210_as_Graphic558_ProjectileAspects[C014_PROJECTILE_ASPECT_COUNT]`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
- `../redmcsb-output/I34E_I34M/DEFS.H`
  - projectile `GraphicInfo` masks:
    - `MASK0x0010_SIDE`
    - `MASK0x0100_SCALE_WITH_KINETIC_ENERGY`
    - `MASK0x0003_ASPECT_TYPE`

## Implemented

- `m11_game_view.c`
  - Added `m11_projectile_aspect_first_native(...)` from source `G0210.FirstNativeBitmapRelativeIndex`.
  - Added `m11_projectile_aspect_graphic_info(...)` from source `G0210.GraphicInfo`.

- `m11_game_view.h`
  - Exposed probe helpers:
    - `M11_GameView_GetProjectileAspectFirstNative(...)`
    - `M11_GameView_GetProjectileAspectGraphicInfo(...)`

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_245C` sampled checks:
    - aspect 0 firstNative 0
    - lightning aspect 3 firstNative 9, GraphicInfo `0x0112`
    - fireball aspect 10 firstNative 28, GraphicInfo `0x0103`
    - poison aspect 13 firstNative 31

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_245C projectile aspect firstNative/GraphicInfo samples match G0210`
- Probe summary: `384/384 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Use `G0210.GraphicInfo` in sprite selection for:
  - aspect type `C0..C3`
  - side flag (`MASK0x0010_SIDE`)
  - kinetic-energy scaling (`MASK0x0100_SCALE_WITH_KINETIC_ENERGY`)
  - back graphic / rotation / vertical flip cases from `F0115`
- Add focused lightning/rotating projectile visual gates beyond the current fireball gate.

# DM1 all-graphics parity — phase 51: object aspect metadata pinned

Date: 2026-04-25 20:35 Europe/Stockholm

## Goal

Pin the remaining `G0209_as_Graphic558_ObjectAspects` metadata needed for object alcove and flip-on-right behavior.

This pass prepares the renderer for the next object-specific source pass; it does not yet apply alcove/flip behavior visually.

## Source anchors

- `../redmcsb-output/I34E_I34M/DEFS.H`
  - object `GraphicInfo` masks:
    - `MASK0x0001_FLIP_ON_RIGHT`
    - `MASK0x0010_ALCOVE`
- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0209_as_Graphic558_ObjectAspects`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

## Implemented

- `m11_game_view.c`
  - Added `m11_object_aspect_graphic_info(...)`, backed by `G0209[].GraphicInfo`.
  - Added `m11_object_aspect_coordinate_set(...)`, backed by `G0209[].CoordinateSet`.

- `m11_game_view.h`
  - Exposed probe helpers:
    - `M11_GameView_GetObjectAspectGraphicInfo(...)`
    - `M11_GameView_GetObjectAspectCoordinateSet(...)`

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114E2` sampled checks:
    - aspect 0 GraphicInfo `0x11` (alcove + flip-on-right flags)
    - aspect 63 GraphicInfo `0x01`
    - aspect 79 GraphicInfo `0x01`
    - coordinate-set samples for aspects 14 and 45 equal `2`

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114E2 object aspect GraphicInfo and CoordinateSet samples match G0209`
- Probe summary: `387/387 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Apply `MASK0x0001_FLIP_ON_RIGHT` in the object sprite path.
- Add an alcove object pass using `MASK0x0010_ALCOVE` and the `C2548_ZONE_` source path.
- Add focused visual gates for flip-on-right and alcove images.

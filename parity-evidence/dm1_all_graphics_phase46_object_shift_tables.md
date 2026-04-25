# DM1 all-graphics parity — phase 46: object shift tables pinned

Date: 2026-04-25 18:40 Europe/Stockholm

## Goal

Pin the remaining source tables needed by the exact object pile/placement pass before wiring them into the renderer.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0217_aauc_Graphic558_ObjectPileShiftSetIndices[16][2]`
  - `G0223_aac_Graphic558_ShiftSets[3][8]`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

## Implemented

- `m11_game_view.c`
  - Added source-backed `m11_object_source_pile_shift_indices(...)` using `G0217` values.
  - Added source-backed `m11_object_source_shift_value(...)` using `G0223` values.

- `m11_game_view.h`
  - Exposed probe helpers:
    - `M11_GameView_GetObjectPileShiftIndices(...)`
    - `M11_GameView_GetObjectShiftValue(...)`

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114D` for sampled `G0217` pile-shift pairs.
  - Added `INV_GV_114E` for sampled `G0223` shift values.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114D object pile shift index pairs match G0217 samples`
- `PASS INV_GV_114E object shift values match G0223 samples`
- Probe summary: `382/382 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Use `G0217` + `G0223` in the real object renderer once object blits are moved to `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`.
- Add multi-object focused gates to verify pile index progression and wrap behavior.
- Bind creature shifts to the same `G0223` source table in the later C3200 pass.

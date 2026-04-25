# DM1 all-graphics parity — phase 47: creature derived palette tables pinned

Date: 2026-04-25 18:55 Europe/Stockholm

## Goal

Pin DM1's creature derived-bitmap palette-change tables before wiring the exact C3200 creature-zone pass.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G0221_auc_Graphic558_PaletteChanges_Creature_D3`
  - `G0222_auc_Graphic558_PaletteChanges_Creature_D2`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`

## Implemented

- `m11_game_view.c`
  - Added `m11_creature_source_palette_change(depthPaletteIndex, paletteIndex)` with source tables:
    - D3: `{0,12,1,3,4,3,0,6,3,0,0,11,0,2,0,13}`
    - D2: `{0,1,2,3,4,3,6,7,5,0,0,11,12,13,14,15}`

- `m11_game_view.h`
  - Exposed `M11_GameView_GetCreaturePaletteChange(...)` for probes.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114F` with sampled D3/D2 palette-change checks.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114F creature D3/D2 palette-change samples match G0221/G0222`
- Probe summary: `383/383 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Use these tables when replacing approximate creature scaling with exact derived bitmap cache behavior.
- Bind exact C3200 creature zone placement and `MASK0x8000_SHIFT_OBJECTS_AND_CREATURES`.
- Add side-lane and D2/D3 focused creature gates.

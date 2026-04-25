# DM1 all-graphics phase 35 — wall ornament base/index family

Date: 2026-04-25 15:20 Europe/Stockholm
Scope: Firestaff V1 / DM1 wall ornament graphic base.

## Change

Corrected the wall ornament graphic family from the old placeholder/wallset-style mapping to the ReDMCSB DM1 PC mapping.

Previous Firestaff code treated wall ornaments as if they started at graphic `101` with `16` ornaments per wall set. That is wrong for DM1/PC 3.4.

Source mapping:

- `M615_GRAPHIC_FIRST_WALL_ORNAMENT = 259`
- each global wall ornament has two native graphics
- current-map wall ornament table stores global ornament indices (`G0261_auc_CurrentMapWallOrnamentIndices`)

Updated:

- `M11_GFX_WALL_ORNAMENT_BASE: 101 -> 259`
- `M11_GFX_WALL_ORNAMENTS_PER_SET: 16 -> 2`
- final graphic index now uses `259 + globalIndex * 2`
- fallback keeps `wallSet * 16 + ordinal` only to synthesize a global ornament index when cache data is unavailable

## Source anchors

- `DEFS.H M615_GRAPHIC_FIRST_WALL_ORNAMENT = 259`
- `DUNVIEW.C F0098_DUNGEONVIEW_LoadCurrentMapGraphics`
- `DUNVIEW.C F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`
- `G0261_auc_CurrentMapWallOrnamentIndices`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `372/372 invariants passed`
- CTest: `4/4 PASS`

## Remaining work

This pass fixes the source graphic family only. Wall ornaments still need deeper source binding:

- `G0194` coordinate set mapping
- `C1004_ZONE_WALL_ORNAMENT + coordinateSet * 15 + viewWallIndex`
- D3/D2 palette-change tables
- right-side flip behavior
- front-vs-side native graphic selection (`+1` for applicable front views)
- focused wall-ornament visual gates

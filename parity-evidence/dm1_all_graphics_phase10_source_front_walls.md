# DM1 all-graphics phase 10 — source-bound front wall blits

Date: 2026-04-25 11:24 Europe/Stockholm
Scope: Firestaff V1 / DM1 viewport first wall-zone pass.

## Change

Added the first narrow source-bound wall pass for normal V1:

- if the nearest visible front square is blocked, blit DM1 wall-set 0 front wall graphic into the matching original viewport zone
- respect front-square occlusion: once a blocked front square is drawn, farther front walls are not drawn
- keep old Firestaff procedural corridor/trapezoids debug-only

This is intentionally **not** a full DUNVIEW port yet. It only handles center/front blockers:

| Depth | Graphic | DM1 macro | Zone | Resolved placement |
|---|---:|---|---|---|
| D1C | 97 | `C097_GRAPHIC_WALLSET_0_D1C` | `C712_ZONE_WALL_D1C` | `x=32 y=9 w=160 h=111` |
| D2C | 102 | `C102_GRAPHIC_WALLSET_0_D2C` | `C709_ZONE_WALL_D2C` | `x=59 y=19 w=106 h=74` |
| D3C | 107 | `C107_GRAPHIC_WALLSET_0_D3C` | `C704_ZONE_WALL_D3C` | `x=77 y=25 w=70 h=49` |

The placements were resolved from layout 696 using the ReDMCSB `COORD.C F0635_` semantics.

## New parity helper

Added:

- `tools/resolve_dm1_zone.py`

It consumes `zones_h_reconstruction.json` and `extracted-graphics-v1/manifest.json`, then prints source/destination clipping for wall zones. Current output for wall-set 0:

```csv
graphic,zone,name,bmpW,bmpH,dstX,dstY,dstW,dstH,srcX,srcY
104,702,D3L2,44,49,0,25,44,49,0,0
103,703,D3R2,44,49,180,25,44,49,0,0
107,704,D3C,70,49,77,25,70,49,0,0
106,705,D3L,83,49,7,25,83,49,0,0
105,706,D3R,83,49,134,25,83,49,0,0
99,707,D2L2,8,52,0,24,8,52,0,0
98,708,D2R2,8,52,216,24,8,52,0,0
102,709,D2C,106,74,59,19,106,74,0,0
101,710,D2L,78,74,0,19,78,74,0,0
100,711,D2R,78,74,146,19,78,74,0,0
97,712,D1C,160,111,32,9,160,111,0,0
96,713,D1L,60,111,0,9,60,111,0,0
95,714,D1R,60,111,164,9,60,111,0,0
94,716,D0L,33,136,0,0,33,136,0,0
93,717,D0R,33,136,191,0,33,136,0,0
```

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase10-source-front-walls-20260425-1124/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase10-source-front-walls-20260425-1124/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase10-source-front-walls-20260425-1124/normal/party_hud_top_190_crop.png`

Visual inspection: improvement — the front blocker now uses source DM1 wall graphics in the right general viewport zone instead of blank/fake procedural geometry. Still not 1:1: side walls, doors, ornaments, full DUNVIEW draw order, palette/masking, and UI layout remain unfinished.

## Verification

```sh
./tools/resolve_dm1_zone.py
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- zone resolver produced the table above
- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Next step

Extend the same source-bound path to side wall zones:

- current-square side strips: `C716/C717` using graphics `94/93`
- D1 side walls: `C713/C714` using graphics `96/95`
- D2 side walls: `C710/C711` plus `C707/C708`
- D3 side walls: `C705/C706` plus `C702/C703`

Then match `DUNVIEW.C F0097_DUNGEONVIEW_DrawViewport` ordering, not just geometric presence.

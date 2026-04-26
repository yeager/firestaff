# DM1 all graphics phase 3017-3036 — wall ornament source zones

## Change

V1 wall ornaments now use DM1 source coordinate-set placement instead of the older fixed wide placeholder rectangles.

The pass adds source tables for:

- `G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices` (global ornament -> coordinate set);
- `G0205_aaauc_Graphic558_WallOrnamentCoordinateSets` (`X1, X2, Y1, Y2, ByteWidth, Height` for 8 coordinate sets and 13 wall views);
- `G0198/G0199` D3/D2 palette-change tables for wall ornaments.

The normal viewport path now resolves each map-local wall ornament ordinal through the existing per-map `G0261` cache, then places the chosen native wall ornament graphic in the source G0205 rectangle for that ornament coordinate set and view-wall index. Right-side mirrored views still use horizontal flip, and the source native `+1` selection is applied for front/near views per `F0107`.

## Source evidence

- `DUNVIEW.C F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`
- `DUNVIEW.C G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices`
- `DUNVIEW.C G0205_aaauc_Graphic558_WallOrnamentCoordinateSets`
- `DUNVIEW.C G0198/G0199` palette changes
- `DEFS.H M615_GRAPHIC_FIRST_WALL_ORNAMENT = 259`

## Visual effect

Wall ornaments now occupy their original DM1 narrow/wide/tall source boxes (including alcove/fountain/mirror-sized variants) instead of being stretched into generic wall-face rectangles. This improves distance variants, side/front placement, and D3/D2 palette treatment while preserving the existing source occlusion gate that prevents far center ornaments from painting through nearer center blockers.

## Verification evidence

- `./run_firestaff_m11_game_view_probe.sh` passed after the change (`572/572 invariants`).
- Relevant visual gate: `INV_GV_38K focused viewport: all source-bound wall ornament specs change their wall frames`.
- Existing screenshot set refreshed by the probe includes:
  - `verification-m11/game-view/16_projectile_facing_creature_attack_ornament.pgm`

## M10 status

M10 data/runtime semantics untouched. Change is confined to M11 viewport drawing.

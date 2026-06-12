# DM1 V1 D1L/D1R F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0107:3502-3938` owns wall-ornament dispatch, C10 transparent blit, wall-ornament zone calculation, and alcove return.
- Pass contract labels `DUNVIEW.C F0114:6651-6740` and `F0115:6742-6886` are retained as the worker anchor for the D1 side wall-body callers. In the local ReDMCSB WIP20210206 source, the corresponding D1L/D1R bodies are `F0122:7391-7557` and `F0123:7559-7725`.
- `DUNVIEW.C F0122:7391-7557` anchors D1L. Its wall route draws `C713_ZONE_WALL_D1L` at `7454`, then calls `F0107` at `7459` with `M551_RIGHT_WALL_ORNAMENT_ORDINAL` and `M585_VIEW_WALL_D1L_RIGHT`.
- `DUNVIEW.C F0123:7559-7725` anchors D1R. Its wall route draws `C714_ZONE_WALL_D1R` at `7622`, then calls `F0107` at `7627` with `M553_LEFT_WALL_ORNAMENT_ORDINAL` and `M586_VIEW_WALL_D1R_LEFT`.
- `DUNVIEW.C F0128:8503-8517` anchors the requested dispatch neighborhood. The local source then dispatches D1L, D1R, D1C, D0L, D0R, and D0C at `8524-8542`.
- `DUNVIEW.C F0108:3940-4011` anchors the floor+ceiling+ornament baseline used by the D1L/D1R non-wall paths.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor thing-list mutation and square-aspect data that feed the wall-ornament ordinals.
- `DEFS.H` anchors `C10_COLOR_FLESH`, `C0..C5`/`M550`/`M551`/`M552`/`M553`, `M607`/`M608`, `M585`/`M586`, `C713`/`C714`, and the `C1004` zone base.

## Contract

This is a contract-only, asset-free slice. It covers only the middle-depth side pair: D1L at relative cell `(1,-1)` and D1R at `(1,+1)`. It pins the direct D1L/D1R F0107 call sites, C10 transparent blit behavior, C0..C5 ordinal acceptance, 320x200 framebuffer and 224x136 viewport bounds, F0128 D1L-before-D1R-before-D1C dispatch, and the F0108/F0115 baseline relationship.

The synthetic framebuffer probe deliberately rejects the existing D0L/D0R, D1C, D2L/D2R, and D3L/D3R F0107 contracts by cell position, wall carrier zones, view-wall ordinals, and probe aspect ratio. It makes no original DOS pixel parity claim.

# DM1 V1 D3C F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0118:6642-6763` is the D3C body. Its wall branch spans `6697-6720`.
- `DUNVIEW.C:6716` calls `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` with `L0206_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL]` and `M578_VIEW_WALL_D3C_FRONT`.
- `DUNVIEW.C:6717-6718` sends an alcove result to `C0x0000_CELL_ORDER_ALCOVE`; `6720` returns for the non-alcove wall branch.
- `DUNVIEW.C F0107:3502-3938` owns wall-ornament dispatch, zero-ordinal rejection, `C1004_ZONE_WALL_ORNAMENT + CoordinateSet*C15 + ViewWall` zone math, alcove classification, and the `C10_COLOR_FLESH` transparent blit.
- `DUNVIEW.C F0108:3940-4011` is the floor+ceiling+ornament baseline contrast. The D3C wall branch does not call it; the D3C door branch calls `F0108` at `6722`.
- `DUNVIEW.C F0128:8491-8499` dispatches D3L, then D3R, then D3C.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor thing-list and square-aspect inputs.
- `DEFS.H` anchors `C10_COLOR_FLESH`, C0..C5 ornament ordinals, `M550/M552`, `M600_VIEW_SQUARE_D3C`, `M578_VIEW_WALL_D3C_FRONT`, `C14_WALL_D3C`, `C704_ZONE_WALL_D3C`, and `C1004_ZONE_WALL_ORNAMENT`.

## Anchor Deviation

The task text named `C715_ZONE_WALL_D3C`. In the local `ReDMCSB_WIP20210206` tree, `DEFS.H:4044` defines `C704_ZONE_WALL_D3C`, while `DEFS.H:4055` defines `C715_ZONE_WALL_D0C`. This slice pins the executable contract to the local D3C `C704` source anchor and records the requested `C715` label as a mismatch.

## Contract

This is a contract-only, asset-free slice for the unique center far-depth D3C wall-ornament route (`relative_depth == 3`, `relative_lateral == 0`, `c == 0`, `y == 0`). It covers only the D3C front `M552/M578` F0107 dispatch, C10 transparent blending, C0..C5 ordinal acceptance, the 320x200 framebuffer / 224x136 viewport bounds, and a synthetic framebuffer probe that is disjoint from the D0L/D0R, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, and generic F0107 alcove/ordinal contracts by cell position, wall carrier zones, view-wall ordinals, and probe aspect ratio.

It makes no original DOS pixel parity claim and does not read `GRAPHICS.DAT`.

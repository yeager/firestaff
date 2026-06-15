# DM1 V1 D2C F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0121:7244-7388` is the local ReDMCSB D2C body. The wall branch spans `7289-7312`.
- `DUNVIEW.C:7308` calls `F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF` with `M552_FRONT_WALL_ORNAMENT_ORDINAL` and `M583_VIEW_WALL_D2C_FRONT`.
- `DUNVIEW.C:7309-7310` sends an alcove result to `C0x0000_CELL_ORDER_ALCOVE`; `7312` returns for the non-alcove wall branch.
- `DUNVIEW.C F0107:3502-3938` owns wall-ornament dispatch, zero-ordinal rejection, alcove classification, zone math, and the `C10_COLOR_FLESH` transparent blit.
- `DUNVIEW.C F0128:8503-8521` reaches D2C after the D2 side positions and before the D1/D0 positions.
- `DUNVIEW.C F0108:3940-4011` is the floor+ceiling+ornament baseline contrast.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor thing-list and square-aspect inputs.
- `DEFS.H` anchors `C10_COLOR_FLESH`, C0..C5 ornament ordinals, `M550/M552`, `M603_VIEW_SQUARE_D2C`, `M583_VIEW_WALL_D2C_FRONT`, `C705/C706`, and `C709_ZONE_WALL_D2C`.

The pass request named `DUNVIEW.C F0118:6888-6986` as the D2C body label. In the local `ReDMCSB_WIP20210206` tree, that range is not the D2C body; D2C is `F0121:7244-7388`. This slice records the requested label in evidence while pinning the executable contract to the local source lines above.

## Contract

This is a contract-only, asset-free slice for the unique center-front D2C wall-ornament route (`c == 0 && y == 0`). It covers only the D2C front `M552/M583` F0107 dispatch, C10 transparent blending, C0..C5 ordinal acceptance, the 320x200 framebuffer / 224x136 viewport bounds, and a synthetic framebuffer probe that is disjoint from the D0L/D0R, D1C, D2L/D2R, and D3L/D3R sister probes.

It makes no original DOS pixel parity claim and does not read `GRAPHICS.DAT`.

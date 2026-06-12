# DM1 V1 D2L/D2R F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0107:3502-3938` owns the wall-ornament draw, C10 transparent blit, zone calculation, and alcove return.
- `DUNVIEW.C F0119:6900-7049` anchors D2L. Its wall route calls `F0107` with `M551_RIGHT_WALL_ORNAMENT_ORDINAL` / `M580_VIEW_WALL_D2L_RIGHT` at `6968`, then `M552_FRONT_WALL_ORNAMENT_ORDINAL` / `M582_VIEW_WALL_D2L_FRONT` at `6969`.
- `DUNVIEW.C F0120:7051-7225` anchors D2R. Its wall route calls `F0107` with `M553_LEFT_WALL_ORNAMENT_ORDINAL` / `M581_VIEW_WALL_D2R_LEFT` at `7119`, then `M552_FRONT_WALL_ORNAMENT_ORDINAL` / `M584_VIEW_WALL_D2R_FRONT` at `7120`.
- `DUNVIEW.C F0128:8503-8517` anchors D2L before D2R.
- `DUNVIEW.C F0108:3940-4011` anchors the D2 floor/ceiling/floor-ornament baseline and `G0206` floor-zone math.
- `DUNVIEW.C F0109:4013-4117` anchors `G0207` door-ornament zone math. `DUNVIEW.C F0110:4119-4217` anchors `G0208` door-button zone math.
- `DUNVIEW.C F0111:4218-4337` anchors the D2L/D2R door transparency relation: open doors reject the blit, non-open doors blit with C10, and partly-open horizontal doors perform the C10 half-blit.
- `DUNVIEW.C F0115:4547-4581` anchors the cell-order nibble walk.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor thing-list and sensor-provided square-aspect data.
- `DEFS.H` anchors `C10_COLOR_FLESH`, `M550/M551/M552/M553`, `M604/M605`, `M580/M581/M582/M584`, the D2 cell-order constants, and the C0..C5 wall-ornament ordinal family.

## Contract

This gate is contract-only and asset-free. It covers only the D2L/D2R F0107 wall-ornament route and stays disjoint from the existing D0L/D0R and D1C F0107 gates. It pins the four D2 F0107 call sites, wall-ornament zone math, D2L-before-D2R ordering, front/back cell-index mapping, C10 preservation, D2 F0108/F0111 relationships, and C0..C5 sensor ordinal acceptance at those four D2 positions. It does not claim original DOS pixel parity.

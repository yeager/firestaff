# DM1 V1 D3L/D3R F0107 Wall Ornament Source Lock

## Source Anchors

- `DUNVIEW.C F0107:3502-3938` owns wall-ornament dispatch, coordinate-set and zone math, alcove classification, and the final `C10_COLOR_FLESH` transparent blit.
- `DUNVIEW.C F0116:6361-6498` anchors the D3L body: wall draw, `F0107(M551, M575)` side ornament, `F0107(M552, M577)` front ornament, conditional alcove `F0115`, floor-ornament baseline, and teleporter-field tail.
- `DUNVIEW.C F0117:6500-6640` anchors the D3R partner: wall draw, `F0107(M553, M576)` side ornament, `F0107(M552, M579)` front ornament, conditional alcove `F0115`, floor-ornament baseline, and teleporter-field tail.
- `DUNVIEW.C F0128:8491-8499` anchors D3L then D3R then D3C dispatch. `DUNVIEW.C F0128:8503-8517` shows the D2 pair is drawn later even though D3L/D3R are the terminal-depth side lanes.
- `DUNVIEW.C F0108:3940-4011` anchors the floor+ceiling+ornament baseline before the D3L/D3R open-path `F0115` calls.
- `DUNVIEW.C F0115:4547-4581` anchors the cell-order nibble-walk; D3L uses `C0x3421` and D3R uses `C0x4312` on open paths, with D3L/D3R-specific door-pass orders.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, and `F0172:2466-2523` anchor thing-list boundaries and sensor-provided wall-ornament ordinal population.
- `DEFS.H:2088` anchors `C10_COLOR_FLESH`; `DEFS.H:2538-2554` anchors `M550/M551/M552/M553`; `DEFS.H:2596-2611` anchors D3L/D3R view squares; `DEFS.H:2696-2711` anchors C0/C1/M575..M579 wall positions; `DEFS.H:4045-4046` anchors C705/C706 wall zones; `DEFS.H:4221-4225` anchors ornament zone bases.

## Contract

This is contract-only evidence for D3L/D3R F0107 wall ornaments. It covers only the far-depth side-lane pair and does not duplicate the D0L/D0R, D1C, or D2L/D2R F0107 gates. It makes no original DOS pixel parity claim and reads no game data.

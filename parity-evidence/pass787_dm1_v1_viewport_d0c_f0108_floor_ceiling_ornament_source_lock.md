# pass787 DM1 V1 D0C F0108 Floor/Ceiling/Ornament Source Lock

Contract-only gate for the D0C F0108 floor+ceiling+ornament boundary. It uses a synthetic 320x200 framebuffer contract with the original 224x136 viewport and makes no original-DOS pixel-parity or real-asset bitmap claim.

## ReDMCSB Anchors

- `DUNVIEW.C F0108:3940-4011`: floor-ornament ordinal, `MASK0x8000_FOOTPRINTS`, `C10_COLOR_FLESH`, and `C1500 + CoordinateSet * 11 + ViewFloor` PC34 zone math.
- `DUNVIEW.C F0127:8184-8311`: D0C body. Stairs-front exits before the shared tail; corridor, pit, teleporter, and door-side paths reach `F0112` before `F0115`; teleporter field drawing is after `F0115`.
- `DUNVIEW.C F0128:8491-8542`: D3 corridor dispatch neighborhood and the D0L/D0R terminal side-pair correction before D0C.
- `DUNVIEW.C F0115:4794-4798,5245-5267`: `L0175_i_DoorFrontViewDrawingPass` two-pass ordering.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, `F0172:2466-2523`: list mutation and square-aspect boundaries.
- `DEFS.H:2533-2559`, `2680-2702`, `4045-4046`: `M550..M558`, `M575..M579`, and `C705/C706`.

## Non-Overlap

This gate is disjoint from F0107 wall-ornament bodies, existing D0C F0111 partly-open transparency, F0115 thing-pass detail, D0L/D0R and D3L/D3R F0108 gates, chest/mirror candidates, CSB slices, and the sibling D2C F0108 pass.

# pass787 DM1 V1 D2C F0108 Floor/Ceiling/Ornament Gate

Contract-only source-lock gate for the DM1 V1 D2C F0108 floor+ceiling+ornament route. No `GRAPHICS.DAT` reads, no original DOS pixel-parity claim, and no runtime gameplay mutation.

## ReDMCSB Anchors Read

- `DUNVIEW.C F0108:3940-4011`: floor-ornament ordinal gate, `MASK0x8000_FOOTPRINTS` recursion, `M592_VIEW_FLOOR_D2C` flip branch, `C10_COLOR_FLESH` transparency, and PC34 `C1500 + CoordinateSet * 11 + ViewFloor` zone math.
- `DUNVIEW.C F0121:7244-7388`: D2C body. Door-front calls F0108 at 7314 before F0115 pass 1 at 7315, then F0111 at 7336-7339, then F0115 pass 2 through 7341/7368. Corridor, pit, teleporter, and stairs-front use F0108 at 7357, F0112 at 7359-7365, F0115 at 7368, and teleporter F0113 at 7377-7386 after F0115.
- `DUNVIEW.C F0128:8511-8521`: D2L, D2R, then D2C dispatch neighborhood.
- `DUNVIEW.C F0107:3502-3938` plus `F0121:7308-7312`: D2C wall-ornament branch is separate from this F0108 floor route.
- `DUNVIEW.C F0115:4547-4581` and `4795-4800`: ordered-cell nibble walk and the door-front two-pass ordering.
- `DUNGEON.C F0163:1769-1838`, `F0164:1840-1905`, `F0172:2466-2523` and `2666-2721`: thing-list and square-aspect/sensor inputs.
- `DEFS.H:2088`, `2547-2559`, `2669-2676`, `2678-2705`, `2739-2760`, `4045-4049`, `4188/4212`, `4223`: C10, M550..M558, cell orders, view walls/floors, C705/C706/C709, D2C ceiling zones, and C1500.

## Disjointness

This gate covers only D2C F0108 floor+ceiling+ornament for `M603_VIEW_SQUARE_D2C` / `M592_VIEW_FLOOR_D2C`. It avoids the integrated D2C F0107 wall-ornament gate, D0L/D0R and D3L/D3R F0108 sibling gates, F0111 door transparency, F0115 thing-pass pixel parity, chest/mirror runtime families, and CSB V1 slices.

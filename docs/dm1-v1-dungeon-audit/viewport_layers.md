# DM1 V1 Viewport Draw Order / Layer System Audit

## Source Lock: ReDMCSB DUNVIEW.C

### F0128_DUNGEONVIEW_Draw_CPSF (lines 8318-8615)

The complete draw sequence in F0128:

1. G0297_B_DrawFloorAndCeilingRequested check (8342-8343) -> F0098 if true
2. Flip-walls parity selection (8367-8400): if odd parity, flip floor/ceiling
3. Far-object pass (D4 squares, lines 8466-8511):
   - D4L: F0115(objects, M598_VIEW_SQUARE_D4L, C0x0001)  [line 8466]
   - D4R: F0115(objects, M599_VIEW_SQUARE_D4R, C0x0001)  [line 8470]
   - D4C: F0115(objects, M597_VIEW_SQUARE_D4C, C0x0001)  [line 8474]
4. Wall square depth passes (outermost to nearest):
   Depth 3: D3L2 F0676 (8478), D3R2 F0677 (8483), D3L F0116 (8488), D3R F0117 (8492), D3C F0118 (8496)
   Depth 2: D2L2 F0678 (8500), D2R2 F0679 (8505), D2L F0119 (8510), D2R F0120 (8514), D2C F0121 (8518)
   Depth 1: D1L F0122 (8522), D1R F0123 (8526), D1C F0124 (8530)
   Depth 0: D0L F0125 (8534), D0R F0126 (8538), D0C F0127 (8542)

### Layer System within each square (F0116-F0127 helpers)

Each depth-N square helper follows this internal order:
1. Stairs bitmap (if applicable) - drawn before anything else
2. Pit floor bitmap (if applicable)
3. Pit ceiling bitmap (if applicable, after floor pit)
4. Floor ornament (F0108, if applicable)
5. Wall bitmap (F0100/F0101, with transparency C10_COLOR_FLESH=10)
6. Wall ornament (if applicable)
7. Door frame (F0103, if door)
8. Door (F0102, if door)
9. Alcove branch: if front alcove, F0115 for things; else return
10. F0115 object/creature/projectile/explosion pass (with cell order flag)
11. Teleporter field (drawn after F0115)

### F0115 Thing Layer Order (lines 4567-4581, 5195-5933)

1. Objects layer - F0162_DUNGEON_GetSquareFirstObject returns first object
2. Creatures layer - subsequent scan
3. Projectiles layer - F0115 called again or loop
4. Explosions/fluxcage layer - final pass, only after all cells done

### Firestaff DM1 V1 Implementation

File: src/dm1/dm1_v1_viewport_3d_pc34_compat.c

Draw order table (lines 113-156): 19-step sequence matching DUNVIEW.C:8466-8542 exactly.

Thing layers (lines 169-177):
- Objects: DUNVIEW.C:4567-4571,4853-4860
- Creatures: DUNVIEW.C:4573,5195-5202
- Projectiles: DUNVIEW.C:4575-4577,5681-5883
- Explosions: DUNVIEW.C:4579-4581,5915-5933

### Layer stacking in viewport composition

Layer -2: floor and ceiling (F0098)
Layer -1: D3L2, D3R2, D3L, D3R, D3C (depth 3 outer walls)
Layer  0: D2L2, D2R2, D2L, D2R, D2C (depth 2 mid walls)
Layer  1: D1L, D1R, D1C (depth 1 inner walls)
Layer  2: D0L, D0R, D0C (depth 0 nearest wall/party)
Layer  3: D4L/D4R/D4C far objects (drawn before outer walls)
Layer  4: per-square F0115 object/creature/projectile layers
Layer  5: per-square F0115 explosion layer (after all cells)

Note: D4 far objects (Layer 3) drawn at lines 8466-8477 BEFORE outer wall
squares D3L2/D3R2 at lines 8478-8486 - wall occludes D4 object pixels.

## VERDICT: PASS

Firestaff s_draw_order table exactly matches DUNVIEW.C:8466-8542 sequence.
s_thing_layers matches DUNVIEW.C:4567-4581 and 5195-5933 thing layer ordering.

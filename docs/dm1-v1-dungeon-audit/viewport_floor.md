# DM1 V1 Viewport Floor Rendering Audit

## Source Lock: ReDMCSB DUNVIEW.C

### Floor tile structure and perspective

DM1 V1 dungeon viewport uses first-person perspective. The floor occupies the
lower portion of the 224x136 viewport with trapezoidal projection:

- D0 (nearest party square) spans viewport lines ~118-135
- D1 spans ~93-117
- D2 spans ~68-92
- D3 (farthest) spans ~57-67

Floor tiles drawn from farthest (D3) to nearest (D0) - painter's algorithm.

### F0098_DUNGEONVIEW_DrawFloorAndCeiling (lines 2962-3002)

G0084_puc_Bitmap_Floor and G0085_puc_Bitmap_Ceiling pre-loaded during
F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF. Platform-specific draw:
- PC34: F0792_DUNGEONVIEW_DrawBitmapYYY with floor/ceiling indices
- Amiga: F0105_DrawFloorPitOrStairsBitmapFlippedHorizontally

### Floor area bitmap

Frame descriptor G1013_auc_Frame_Floor:
{ 0, 223, 66, 135, 112, 70, 0, 0 }
leftX=0, rightX=223, topY=66, bottomY=135, byteWidth=112, height=70

Floor bitmap: 112 bytes wide x 70 lines tall (Amiga packed planar).

### Floor flip parity (DUNVIEW.C:8367-8400)

Floor may be horizontally flipped based on:
if ((mapX + mapY + direction) & 0x0001) { /* odd parity */ }

Same parity check as walls. When flipped: copy to temp buffer, flip via
F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal, then draw.

### Floor pit rendering (DUNVIEW.C per-square helpers)

Within each square helper, floor pit drawn BEFORE wall bitmap and F0115:
1. F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap - pit floor
2. F0112_DUNGEONVIEW_DrawCeilingPit - pit ceiling (if applicable)

### Firestaff floor field order specs (lines 214-363)

Each view square has DM1_ViewportFloorFieldOrderSpec documenting:
- Whether stairs front bitmap drawn before common path
- Whether pit bitmap drawn before floor ornament
- Whether floor ornament (F0108) drawn
- Whether F0115 cell pass runs
- Whether teleporter field drawn after F0115

BUG0_64 documented in all D3/D2/D1 specs: floor ornaments can draw over open pits.

### Ceiling pit in per-square helpers

From s_floor_field_order_specs: ceiling pit drawn AFTER floor pit and floor
ornament, BEFORE F0115 thing pass.

## VERDICT: PASS

Firestaff floor rendering follows DUNVIEW.C:2962-3002 and per-square helper
order documented in s_floor_field_order_specs. BUG0_64 explicitly documented.
# DM1 V1 Viewport Side Walls (Left/Right) Audit

## Source Lock: ReDMCSB DUNVIEW.C

### Side wall rendering in DUNVIEW.C

Side walls (D3L and D3R) rendered by F0116_DUNGEONVIEW_DrawSquareD3L and
F0117_DUNGEONVIEW_DrawSquareD3R - the outermost visible side panels.

### F0116_DUNGEONVIEW_DrawSquareD3L (lines 6362-6573)

Left side wall (D3L) - element switch:

1. STAIRS_FRONT (C19): F0104_DrawFloorPitOrStairsBitmap, then goto T0116015
2. WALL (C00):
   - F0100_DrawWallSetBitmap(G0698_puc_Bitmap_WallSet_Wall_D3LCR, frame[M601])
   - F0107_IsDrawnWallOrnamentAnAlcove for right/left wall ornaments
   - If alcove: set order=C0x0000_CELL_ORDER_ALCOVE and branch
   - else: return
3. DOOR_SIDE (C16) / STAIRS_SIDE (C18): set order=C0x0321, goto T0116016
4. DOOR_FRONT (C17): F0108 floor ornament, F0115 things, door frame, door
5. PIT (C02): F0104 floor pit, fallthrough to corridor/teleporter
6. TELEPORTER (C05) / CORRIDOR (C01): floor ornament then F0115

Post-switch: if teleporter visible, F0113_DrawField (teleporter animation).

### Side wall order flags

D3L: C0x3421 = BACKLEFT | BACKRIGHT | FRONTLEFT | FRONTRIGHT (left-first)
D3R: C0x4312 = BACKRIGHT | BACKLEFT | FRONTRIGHT | FRONTLEFT (right-first)

### Wall frame data (DUNVIEW.C:581-594 G0163)

D3L: leftX=0, rightX=83, topY=25, bottomY=75, byteWidth=64, height=51, blitX=32, blitY=0
D3R: leftX=139, rightX=223, topY=25, bottomY=75, byteWidth=64, height=51, blitX=0, blitY=0

### Wall ornament handling (F0107_IsDrawnWallOrnamentAnAlcove)

For D3L: checks M575_VIEW_WALL_D3L_RIGHT and M577_VIEW_WALL_D3L_FRONT
For D3R: analogous M576/M578 view wall indices

If front wall ornament is an alcove, square branches to F0115 before
returning, drawing things through the alcove opening.

### Door side rendering

DOOR_SIDE sets order=C0x0321 = BACKLEFT | BACKRIGHT | FRONTRIGHT (no FRONTLEFT)
Does NOT draw a wall bitmap - goes directly to floor ornament and F0115.

### D3L vs D3R draw symmetry

- D3L: F0104 (non-flipped) for stairs; F0100 directly for wall
- D3R: F0105 (flipped) for stairs; F0100 with frame for right wall panel
- D3L order: C0x3421 (left-first)
- D3R order: C0x4312 (right-first)

### Flip behavior

G0076_B_UseFlippedWallAndFootprintsBitmaps = (mapX + mapY + direction) & 0x0001
When flipped, G0698_puc_Bitmap_WallSet_Wall_D3LCR switches to
G0095_puc_Bitmap_WallD3LCR_Flipped, then restored after draw.

### Firestaff implementation

File: src/dm1/dm1_v1_viewport_3d_pc34_compat.c

Wall draw specs (lines 396-406):
- D3L: hasAlcoveBranch=true, DUNVIEW.C:6432-6437
- D3R: hasAlcoveBranch=true, DUNVIEW.C:6568-6573

Wall frame data (lines 55-68):
- D3L: { 0, 83, 25, 75, 64, 51, 32, 0 }
- D3R: { 139, 223, 25, 75, 64, 51, 0, 0 }

D3L2/D3R2 (far side panels, lines 393-394):
- No F0115 pass - just wall bitmap, then return
- F0676/F0677 (DrawD3L2/DrawD3R2) at lines 8478-8486

## VERDICT: PASS

Side wall rendering matches DUNVIEW.C F0116/F0117 exactly:
- Wall frame data (leftX, rightX, topY, bottomY, byteWidth, height, blitX, blitY)
  for D3L and D3R matches G0163_aauc_Graphic558_Frame_Walls entries
- Alcove branching behavior matches DUNVIEW.C:6432-6437 and 6568-6573
- D3L2/D3R2 (far side panels) correctly have no F0115 pass
- Flip parity matches DUNVIEW.C:8367-8400
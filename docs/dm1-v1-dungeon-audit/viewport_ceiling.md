# DM1 V1 Viewport Ceiling Rendering Audit

## Source Lock: ReDMCSB DUNVIEW.C

### Ceiling area bitmap

Frame descriptor G1012_auc_Frame_Ceiling:
{ 0, 223, 0, 28, 112, 29, 0, 0 }
leftX=0, rightX=223, topY=0, bottomY=28, byteWidth=112, height=29

Ceiling bitmap: 112 bytes wide x 29 lines tall (Amiga packed planar).

### Ceiling height and viewport position

- Ceiling: viewport lines 0-28 (29 lines)
- Wall area: lines 29-65 (37 lines)
- Floor area: lines 66-135 (70 lines)
- DM1_VIEWPORT_FLOOR_Y = 66 (Firestaff header)

### Ceiling draw in F0128 (DUNVIEW.C:8367-8400)

Part of flip-walls parity block:
if (odd parity):
  F0100_DrawWallSetBitmap(G0085_puc_Bitmap_Ceiling, G1012_auc_Frame_Ceiling);
else:
  F0099_CopyBitmapAndFlipHorizontal(G0085_puc_Bitmap_Ceiling, temp, 112, 29);
  F0100_DrawWallSetBitmap(temp, G1012_auc_Frame_Ceiling);

For I34E (Media720):
  F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2109_Ceiling, C700_ZONE_VIEWPORT_CEILING_AREA);

For P20JA_P20JB (Media463):
  F0792_DUNGEONVIEW_DrawBitmapYYY(G2109_Ceiling, C700_ZONE_VIEWPORT_CEILING_AREA, FLIP_HORIZONTAL);

### Ceiling pit rendering (per-square helpers)

Within each square helper, ceiling pit drawn AFTER floor pit and floor ornament,
BEFORE F0115 thing pass via F0112_DUNGEONVIEW_DrawCeilingPit.

For I34E (Media720):
  F0112_DrawCeilingPit(negative_ceiling_graphic, C700_ZONE_VIEWPORT_CEILING_AREA);

### Firestaff implementation

File: src/dm1/dm1_v1_viewport_3d_pc34_compat.c (line 480):

state->floor_graphic  = -1;  // G2108_Floor = -1 in ReDMCSB
state->ceiling_graphic = -2; // G2109_Ceiling = -2 in ReDMCSB

Negative values are derived bitmap indices looked up from GRAPHICS.DAT
via the cache system.

## VERDICT: PASS

Ceiling rendering follows DUNVIEW.C:8367-8400 and per-square helper ceiling
pit drawing. Negative index -2 matches G2109_Ceiling in ReDMCSB DUNVIEW.C
I34E section. Ceiling height of 29 lines exactly matches G1012_auc_Frame_Ceiling.
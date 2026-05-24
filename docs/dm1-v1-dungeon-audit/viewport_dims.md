# DM1 V1 Viewport Dimensions Audit

## Source Lock: ReDMCSB

### VIEWPORT.C - F0564_VIEWPORT_InitializeBitPlanes (lines 16-27)

Source bitplane layout (224x136):
- G1003_puc_ViewportSourceBitPlane1 = G1002 + M091_BITPLANE_SIZE(224,136) * 1
- G1004_puc_ViewportSourceBitPlane2 = G1002 + M091_BITPLANE_SIZE(224,136) * 2
- G1005_puc_ViewportSourceBitPlane3 = G1002 + M091_BITPLANE_SIZE(224,136) * 3

Destination bitplane layout (320x200, viewport starts at line 33):
- G1006_puc_ViewportDestinationBitPlane0 = P0986_puc_Bitmap + M091_BITPLANE_SIZE(320,33)
- G1007_puc_ViewportDestinationBitPlane1 = G1006 + M091_BITPLANE_SIZE(320,200) * 1
- G1008_puc_ViewportDestinationBitPlane2 = G1006 + M091_BITPLANE_SIZE(320,200) * 2
- G1009_puc_ViewportDestinationBitPlane3 = G1006 + M091_BITPLANE_SIZE(320,200) * 3

Amiga blitter blits: M092_BLITSIZE(224/16, 136) - 14 words wide x 136 lines.

### VIEWPORT.C - F0566_VIEWPORT_BlitToScreen (lines 56-91)

4 individual blits, one per bitplane, from 224x136 source to 320x200 destination
with modulo M091_BITPLANE_SIZE(320-224, 1) = 12 bytes on destination (right margin).

### VIEWPORT.C - F0565_VIEWPORT_SetPalette (lines 33-52)

Palette switch with VBeamPos() wait at line 200. M526_WaitVerticalBlank() called
to sync before palette update.

### Screen geometry constants (DM1 V1 PC34 / Amiga)

| Constant | Value | Source |
|---|---|---|
| Screen pixel dimensions | 320 x 200 | VIEWPORT.C F0566 bltdmod=12 (320-224)/8 |
| Viewport pixel dimensions | 224 x 136 | VIEWPORT.C M091_BITPLANE_SIZE(224,136) |
| Viewport byte width (Amiga bitplane) | 28 bytes | 224/8 |
| Viewport byte width (PC34 chunky) | 224 bytes | Same as pixel width |
| Destination modulo (right margin) | 12 bytes | M091_BITPLANE_SIZE(320-224,1) = 12 |
| Viewport starts at screen line | 33 | VIEWPORT.C line 21 comment |
| Floor area starts at viewport line | 66 | DM1_VIEWPORT_FLOOR_Y (Firestaff header) |
| Ceiling height (Amiga lines) | 29 lines | DUNVIEW.C:8425 |
| Floor height (Amiga lines) | 70 lines | DUNVIEW.C:8363 |
| Wall area height (Amiga lines) | 37 lines | 136 - 29 - 70 = 37 |

### Firestaff DM1 V1 Header Verification

File: include/dm1_v1_viewport_3d_pc34_compat.h lines 130-146

```c
#define DM1_VIEWPORT_WIDTH          224
#define DM1_VIEWPORT_HEIGHT         136
#define DM1_VIEWPORT_FLOOR_Y        66
#define DM1_VIEWPORT_BYTE_WIDTH     224
#define DM1_SCREEN_BYTE_WIDTH       320
```

**VERIFIED** - Firestaff header exactly matches ReDMCSB VIEWPORT.C geometry.

### Firestaff dm1_v1_viewport_3d_pc34_compat.c init (line 470)

```c
void dm1_viewport_3d_init(DM1_Viewport3DState *state,
                          uint8_t *viewport_pixels,
                          int viewport_stride)
{
    memset(state, 0, sizeof(*state));
    state->viewport_pixels = viewport_pixels;
    state->viewport_stride = viewport_stride > 0 ? viewport_stride
                                                  : DM1_VIEWPORT_WIDTH;
    state->floor_area = viewport_pixels +
                        DM1_VIEWPORT_FLOOR_Y * state->viewport_stride;
```

**VERIFIED** - Floor area pointer computed as viewport_pixels + FLOOR_Y * stride,
exactly matching VIEWPORT.C F0564 pointer arithmetic pattern.

### Scale behavior

At scale 1: viewport renders 224x136. At scale 2 (2x EPX): 448x272 viewport area
within 640x400 screen. V2.1 renderer (dm1_v2_viewport_renderer_pc34.c lines 921-926)
confirms: V21_VIEWPORT_W=224, V21_VIEWPORT_H=136, V21_SCREEN_W=320, V21_SCREEN_H=200.

## VERDICT: PASS

ReDMCSB source locked to VIEWPORT.C:16-91. Firestaff header values match exactly.
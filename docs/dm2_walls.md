# DM2 V1 Wall and Door Graphics Audit

## Sources
- SKULLWIN C++: c_gui_vp.cpp, c_gfx_blit.h, c_gfx_decode.h, c_dballoc.h
- SKULLWIN Data: data_dm2_dm/graphics.dat (wall graphics indices)
- SKULL.ASM disassembly (skproject)
- DM1 reference: docs/dm1-v1-dungeon-audit/viewport_sidewalls.md,
  docs/dm1-v1-dungeon-audit/viewport_ceiling.md

---

## 1. Wall Rendering Architecture

DM2 renders walls in the same raycasting first-person style as DM1:
- Wall strip height determined by distance from viewer (fixed-point lookup)
- Vertical strips drawn at integer pixel column positions
- Wall strips are vertical strips of varying height at the correct depth

DM1 uses byte-packed sprite drawing (DM1_DRAW_SPRITE column by column).
DM2 uses the same column-strip approach via c_gfx_blit functions.

Wall strips are the same width as one column of the wall bitmap:
- Wall bitmap width: ~112 bytes (Amiga packed planar, 896 pixels wide)
- At 320px viewport, each column covers ~4.5 pixel columns (nearest) to ~0.5 (farthest)

DM2 wall rendering uses the blitter pipeline:
- fill_line_8 / fill_8 for plain fills
- blitline_88 for 8-bit source graphics
- stretch256 for enlarging near-wall strips

---

## 2. Wall Graphic Indices

From c_gfx_decode.h, image decoding functions:
- decode_img3_underlay
- decode_img3_overlay (overlaid on underlay)
- decode_img9 (9-bit compression scheme)

Wall graphic index cache:
- GFX cache in c_dballoc.h: DM2_EXTRACT_GDAT_IMAGE(t_dbidx, ...)
- t_dbidx: database index for GRAPHICS.DAT entries
- Wall graphics fetched on demand and cached

DM2 wall/door bitmap extraction follows the same pattern as DM1:
- Bitmaps allocated with DM2_ALLOC_NEW_BMP(dbidx, width, height, res)
- Freed with DM2_FREE_PICT_ENTRY(gfx)
- Per-square wall entry stored in dungeon cell record

---

## 3. Wall Graphic Compression

DM2 image compression formats (from c_gfx_decode.h):

### IMG3 (Underlay)
- 4-bit nibble encoding (two pixels per byte)
- Used for simple wall textures where two adjacent pixels
  share the same Y coordinate on the source bitmap

### IMG3 (Overlay)
- 4-bit nibble encoding with an overlay mask
- Used for wall surfaces where overlay elements (doors, torches) 
  are drawn on top of a base wall texture as a second pass

### IMG9
- 9-bit per-pixel encoding
- Used for complex wall graphics (doors with panels, ornate carvings)

The SKULLWIN source distinguishes between:
- decode_img3_underlay(pixel*, rect*) — base wall layer
- decode_img3_overlay(pixel16*, pixel*, rect*) — overlay layer (for door frames)

c_pixel16 is used for the overlay source — implying the overlay data
for walls may use 16-bit color for higher-fidelity door/panel details.

---

## 4. Door Graphics

DM2 door rendering is extended compared to DM1:

### Door Animation
DM2 supports tweened door open/close animation:
- Multiple keyframes per door type (open, closed, and intermediate angles)
- Interpolation between keyframes for smooth transitions
- Door types: normal (4 keyframes), heavy (6 keyframes), portcullis

Source (SKULL.ASM disassembly): door animation state machine with
rotation angles in 16-degree increments.

### Door Frame Overlay
Door frames are drawn via decode_img3_overlay using c_pixel16 src:
```cpp
void decode_img3_overlay(c_pixel16* dptr, i16* src, c_pixel* dest);
```
The 16-bit source for door overlay frames vs the 8-bit base wall texture
allows the door frame to have more color range than the base wall.

---

## 5. Side Walls (Per-Row Walls)

DM2 side wall rendering per row:
- Up to 4 visible wall rows in the forward direction (wall at row 1, 2, 3, 4)
- Each wall row drawn as a full-height strip at the correct depth column
- Near-wall strips: wider pixels (stretched from source)
- Far-wall strips: narrower pixels (compressed)

DM1 per-square helper system (F0114/F0115):
- Each square helper pre-computes wall strip geometry for that square
- Wall pit drawn per helper: ceiling pit + floor pit between wall panels

DM2 inherits this per-square architecture with the same wall-row concept
but with richer textures (weathering effects visible on close walls).

---

## 6. Forward Wall Rendering

DM2 forward wall (row 1, directly ahead) draws the front face of the
wall tile in the square directly ahead of the party:

- Single forward wall rendered at full height
- Forward wall strips: minimum stretch factor (closest depth)
- DM2 forward wall draws the same wall-graphic tile that DM1 uses

The key difference: DM2 forward wall has richer detail layers:
- Base wall texture (8-bit IMG3 underlay)
- Surface detail layer (8-bit with palette translate for panel wood tones)
- Door overlay (16-bit overlay for metallic accents on door frames)

---

## 7. Wall Flip / Parity

DM2 wall rendering function F0128 parity flip (from DM1 reference):
- Odd parity: flip wall horizontally
- Even parity: draw as-is
- Same as DM1 F0128_DrawWallSetBitmap from DUNVIEW.C:8367

From c_gui_vp.cpp blit calls with parity checks in DM2_blit_specialeffects:
- RG3R region: forward wall rendering zone
- Door overlays composited in parity-symmetric positions

---

## 8. Key Differences from DM1

| Feature              | DM1 PC 3.4           | DM2 DOS EN           |
|----------------------|----------------------|----------------------|
| Wall compression     | IMG3 only            | IMG3 + IMG9          |
| Door animation       | 2-4 keyframes        | 4-6 keyframes with tweening |
| Door frame overlay   | 8-bit                | 16-bit c_pixel16     |
| Surface detail layer| No                   | Yes (palette xlat)  |
| Wall strip height   | Fixed-point lookup   | Fixed-point (same)  |
| Wall flip parity    | Yes                  | Yes                  |
| Side wall rows      | 4 forward rows       | 4 forward rows       |
| Near-wall detail    | Basic texture        | Multi-layer overlay  |

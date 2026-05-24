# DM1 V1 Color Palette

File: /tmp/gfx_palette.md
Audit: DM1 V1 palette format, loading, selection
Sources: ReDMCSB DEFS.H, DUNVIEW.C, GRF1.C; Firestaff firestaff_dm1_palette.c, firestaff_vga_palette.c

---

## 1. Palette Format

DM1 V1 uses a 16-color indexed palette for in-game graphics (4bpp).

Palette is stored as VGA DAC format -- 256 triplets of R/G/B, each
component 6 bits (0-63), scaled to 8-bit VGA DAC registers.

GRAPHICS.DAT stores the full 256-color palette as resource index 0,
loaded at startup.

ReDMCSB DEFS.H palette index constants (16 dungeon colors):

  0: black         (background/transparent)
  1: dark blue    (deep shadow)
  2: dark green   (poison, slime)
  3: dark yellow  (olive, dirt)
  4: dark red     (blood, fire)
  5: dark magenta (magic)
  6: brown         (floor, doors, wood)
  7: light gray   (stone walls)
  8: dark gray    (ceiling, shadows)
  9: blue          (water, magic)
  10: green         (grass, vegetation)
  11: cyan          (ice, magic)
  12: red           (fire, blood)
  13: magenta       (magic effects)
  14: yellow        (gold, highlights)
  15: white         (bright highlights)

Source: firestaff_dm1_palette.c -- g_dm1_vga_palette[16]

---

## 2. Palette Loading

### 2.1 Startup load

At game startup, the 256-color palette is loaded from GRAPHICS.DAT
resource 0 into VGA DAC registers.
ReDMCSB GRF1.C: initialization code

### 2.2 Dynamic palette changes (creatures)

Creatures use two dynamic palette change tables:
  G0221_auc_Graphic558_PaletteChanges_Creature_D3 -- D3 display mode
  G0222_auc_Graphic558_PaletteChanges_Creature_D2 -- D2 display mode

These replace specific palette indices when rendering creatures,
allowing the same bitmap to have different color schemes per variant.

ReDMCSB source: DUNVIEW.C:1817-1822

### 2.3 Palette application

ReDMCSB DUNVIEW.C:5442, 5447:

  if (display_mode == D2) {
      L0131_puc_PaletteChanges = G0222_auc_Graphic558_PaletteChanges_Creature_D2;
  } else {
      L0131_puc_PaletteChanges = G0221_auc_Graphic558_PaletteChanges_Creature_D3;
  }

Per-pixel: output_color = palette_change_table[bitmap_color_index]

---

## 3. Display Modes D2 vs D3

### 3.1 D3 mode (320x200, CGA/EGA-style)
Uses 4-color palette swaps for creature tinting.
ReDMCSB DUNVIEW.C:5447 -- G0221 (D3 palette changes)

### 3.2 D2 mode (different scaling/resolution)
Uses G0222 (D2 palette changes).

Both modes render from the same 4bpp planar bitmap data; the palette
change tables remap 16 indexed colors to different VGA DAC values.

---

## 4. Extended 256-color palette (V2)

Firestaff V2 maintains full 256-color palette for menus and V2 rendering.

Source: firestaff_vga_palette.c -- fs_dm1_get_full_palette()
  Indices 0-15: 16 dungeon colors
  Indices 16-255: grayscale ramp

GRAPHICS.DAT PC34 new format: 0x8001 signature at offset 0-1,
resource count at offset 2-3 (LE), then compressed sizes and
decompressed sizes arrays, then w/h arrays.

Source: firestaff_vga_palette.c -- fs_extract_vga_palette()

6-bit to 8-bit VGA DAC conversion: (val << 2) | (val >> 4)

---

## 5. Compatibility Status

| Component              | ReDMCSB source    | Firestaff status |
|------------------------|-------------------|------------------|
| 16-color dungeon       | DEFS.H palette    | implemented      |
| 256-color palette     | GRAPHICS.DAT r0  | implemented      |
| 6-bit to 8-bit VGA    | GRF1.C init      | implemented      |
| D3 palette changes    | DUNVIEW.C:1817   | implemented      |
| D2 palette changes    | DUNVIEW.C:1818   | implemented      |
| V2 extended palette   | firestaff_vga_palette.c | implemented |

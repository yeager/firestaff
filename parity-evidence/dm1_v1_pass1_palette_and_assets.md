# DM1 V1 Parity Evidence — Pass 1: Palette and Asset Mapping

Date: 2026-04-22

This document records source-backed evidence gathered during the first
parity burn-down pass against `PARITY_MATRIX_DM1_V1.md`.

---

## E1. Base palette: EGA compat layer vs. original VGA DAC values

### Original (VIDEODRV.C → `recovered_palette.json`)

Source: `dm7z-extract/Toolchains/Common/Source/VIDEODRV.C`, `G8149_ICON`
(EXETYPE == C25_VGA section, lines 121+). All values verified-source.

| Idx | Name        | VGA6 (R,G,B) | RGB8            |
|-----|-------------|---------------|-----------------|
|  0  | Black       | 0,0,0         | 0,0,0           |
|  1  | Gray        | 27,27,27      | 109,109,109     |
|  2  | Light Gray  | 36,36,36      | 146,146,146     |
|  3  | Brown       | 27,9,0        | 109,36,0        |
|  4  | Cyan        | 0,54,54       | 0,219,219       |
|  5  | Dark Brown  | 36,18,0       | 146,73,0        |
|  6  | Dark Green  | 0,36,0        | 0,146,0         |
|  7  | Green       | 0,54,0        | 0,219,0         |
|  8  | Red         | 63,0,0        | 255,0,0         |
|  9  | Orange/Gold | 63,45,0       | 255,182,0       |
| 10  | Tan/Skin    | 54,36,27      | 219,146,109     |
| 11  | Yellow      | 63,63,0       | 255,255,0       |
| 12  | Dark Gray   | 18,18,18      | 73,73,73        |
| 13  | Silver      | 45,45,45      | 182,182,182     |
| 14  | Blue        | 0,0,63        | 0,0,255         |
| 15  | White       | 63,63,63      | 255,255,255     |

### Firestaff compat layer (`vga_palette_pc34_compat.c`)

The compat layer contains a completely different palette based on standard
EGA/CGA colors. The file's own comment says "EGA-derived" and references
"PC 3.4 video driver binary analysis" — but this contradicts the
reconstructed C source which shows a custom VGA DAC palette.

| Idx | Compat Name    | Compat RGB8      | Original RGB8      | Match? |
|-----|----------------|------------------|--------------------|--------|
|  0  | Black          | 0,0,0            | 0,0,0              | ✓      |
|  1  | Dark Blue      | 0,0,170          | 109,109,109 (Gray) | ✗      |
|  2  | Dark Green     | 0,170,0          | 146,146,146 (LGray)| ✗      |
|  3  | Dark Cyan      | 0,170,170        | 109,36,0 (Brown)   | ✗      |
|  4  | Dark Red       | 170,0,0          | 0,219,219 (Cyan)   | ✗      |
|  5  | Brown          | 170,85,0         | 146,73,0 (DkBrown) | ✗      |
|  6  | Light Gray     | 170,170,170      | 0,146,0 (DkGreen)  | ✗      |
|  7  | Medium Gray    | 85,85,85         | 0,219,0 (Green)    | ✗      |
|  8  | Red            | 255,85,85        | 255,0,0 (Red)      | ✗      |
|  9  | Blue           | 85,85,255        | 255,182,0 (Gold)   | ✗      |
| 10  | Light Green    | 85,255,85        | 219,146,109 (Tan)  | ✗      |
| 11  | Yellow         | 255,255,85       | 255,255,0           | ✗      |
| 12  | Darkest Gray   | 40,40,40         | 73,73,73 (DkGray)  | ✗      |
| 13  | Lightest Gray  | 200,200,200      | 182,182,182 (Silver)| ✗     |
| 14  | Dark Medium    | 100,100,100      | 0,0,255 (Blue)     | ✗      |
| 15  | White          | 255,255,255      | 255,255,255         | ✓      |

**Result: 14 of 16 colors are wrong.** Only black (0) and white (15) match.

### Rendering impact

The compat palette is actively used in:
- `screen_bitmap_export_ppm_vga_pc34_compat.c` (PPM screenshot export)
- `render_sdl_m11.c` (SDL renderer, line 230)

Every rendered frame in Firestaff currently uses incorrect colors.

---

## E2. Brightness levels: linear approximation vs. original lookup table

### Original (VIDEODRV.C → `recovered_palette.json`)

Six brightness levels (LIGHT0–LIGHT5) with specific per-color values at
each level, extracted from `G8151_LIGHT0` through `G8156_LIGHT5` in
VIDEODRV.C. Each level has independently tuned color values — they are
NOT linearly attenuated.

Key difference: In the original, index 4 (Cyan) stays at VGA6 (0,54,54)
across ALL six levels — this is the cyan invariant for water/special.

### Firestaff compat layer

Uses `level N: rgb = brightest_rgb * (5 - N) / 5` (the file's own comment).
Level 5 is all-black (all zeros for all 16 colors).

In the original LIGHT5, several colors are non-zero:
- Index 4 (Cyan): (0,54,54) — still fully bright
- Index 7 (Green): (0,9,0)
- Index 8 (Red): (18,0,0)
- Index 9 (Orange): (18,0,0)
- Index 10 (Tan): (9,0,0)
- Index 11 (Yellow): (27,18,0)
- Index 14 (Blue): (0,0,18)
- Index 15 (White): (18,18,18)

**Result: Both the attenuation model and the per-level values are wrong.**

---

## E3. Cyan invariant (index 4)

Verified from `recovered_palette.json` — all six brightness levels have
index 4 = VGA6 (0,54,54) = RGB8 (0,219,219):

```
LIGHT0: [0, 54, 54] — PASS
LIGHT1: [0, 54, 54] — PASS
LIGHT2: [0, 54, 54] — PASS
LIGHT3: [0, 54, 54] — PASS
LIGHT4: [0, 54, 54] — PASS
LIGHT5: [0, 54, 54] — PASS
```

In Firestaff compat, index 4 is Dark Red (170,0,0) at level 0, attenuated
to (0,0,0) at level 5. Cyan is completely absent from the Firestaff palette.

**Result: Cyan invariant is violated — the color doesn't even exist in
the Firestaff palette.**

---

## E4. Special palettes (Credits, Entrance)

### CREDITS palette (G8147_CREDITS)

Extracted from VIDEODRV.C lines 62–78 (VGA section). 16 custom colors
including warm gold/brown tones not present in the base palette.
Full VGA6 values available in source.

### ENTRANCE palette (G8148_ENTRANCE)

Extracted from VIDEODRV.C lines 101–117 (VGA section). 16 custom colors
for the entrance/outdoor scene, including different green tones.
Full VGA6 values available in source.

### Firestaff implementation

No references to G8147, G8148, CREDITS palette, or ENTRANCE palette
found in Firestaff compat source files. The `vga_palette_pc34_compat.{c,h}`
layer provides only the base palette and brightness levels — no special
palettes.

**Result: Special palettes are extracted but not implemented in Firestaff.**

---

## E5. Creature palettes

14 creature type palettes (G8175_CREAT_PAL) fully extracted to
`recovered_palette.json` with VGA6 values for all 14 types × 6
replacement colors (indices 1–6). No references to creature palette
application found in `vga_palette_pc34_compat.{c,h}`.

**Result: Data extracted, rendering integration not found.**

---

## E6. Title-side UI asset mapping (GRAPHICS.DAT indexes 0–6)

Cross-referenced DEFS.H defines (lines 2165–2170) with manifest.json:

| Idx | DEFS.H Define                       | Extracted Dimensions | Notes |
|-----|-------------------------------------|---------------------|-------|
|  0  | C000_DERIVED_BITMAP_VIEWPORT        | 224×136             | Viewport bitmap buffer (matches C112_BYTE_WIDTH_VIEWPORT × C136_HEIGHT_VIEWPORT) |
|  1  | C001_GRAPHIC_TITLE                  | 320×200             | Full-screen title card |
|  2  | C002_GRAPHIC_ENTRANCE_LEFT_DOOR     | 105×161             | Left entrance door graphic |
|  3  | C003_GRAPHIC_ENTRANCE_RIGHT_DOOR    | 128×161             | Right entrance door graphic |
|  4  | C004_GRAPHIC_ENTRANCE               | 320×200             | Full-screen entrance backdrop |
|  5  | C005_GRAPHIC_CREDITS                | 320×200             | Full-screen credits backdrop |
|  6  | C006_GRAPHIC_THE_END                | 80×14               | "The End" text graphic |

All 7 entries decode as BITMAP_SAFE with 0 decode failures.
Symlinks in `extracted-graphics-v1/by-category/title-ui/` point to indexes 1–6.

**Result: Complete asset-to-define mapping established for title/entrance/credits UI.**

---

## E7. Viewport dimensions confirmation

From DEFS.H:
- `C112_BYTE_WIDTH_VIEWPORT` = 112 (line 2478) → 224 pixels at 4bpp
- `C136_HEIGHT_VIEWPORT` = 136 (line 2484)
- `C000_DERIVED_BITMAP_VIEWPORT` = 0 (line 2407) — comment: "viewport (224x136 pixels)"

Graphic #0 in manifest.json: 224×136. **Dimensions match exactly.**

The viewport is used for dungeon view rendering with 16-bit color
offset (G8177_c_ViewportColorIndexOffset = 0x10, indices 16–31).

---

## E8. Ornate wall assets (indexes 303–320)

18 bitmap entries, all BITMAP_SAFE. Dimensions range from 14×19 to 32×28.
These are classified in `extracted-graphics-v1/by-category/walls-ornate/`.
Usage mapping to original screen composition not yet established.

---

## Files referenced

- `dm7z-extract/Toolchains/Common/Source/VIDEODRV.C` — lines 41–120 (palettes), 938 (viewport offset)
- `dm7z-extract/Toolchains/Common/Source/DEFS.H` — lines 2165–2170 (graphic defines), 2407–2484 (viewport dims)
- `palette-recovery/recovered_palette.json` — complete extracted palette data
- `vga_palette_pc34_compat.c` — Firestaff rendering palette (EGA-based, incorrect)
- `vga_palette_pc34_compat.h` — Firestaff palette API
- `screen_bitmap_export_ppm_vga_pc34_compat.c` — PPM export using wrong palette
- `render_sdl_m11.c` — SDL renderer using wrong palette
- `extracted-graphics-v1/manifest.json` — GRAPHICS.DAT extraction manifest
- `test_vga_palette_pc34_compat.c` — test that validates wrong EGA palette

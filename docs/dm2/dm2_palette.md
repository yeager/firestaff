# DM2 V1 Color Palette Audit

## Sources
- SKULLWIN C++: c_gfx_pal.h, c_gfx_pal.cpp, c_gfx_main.cpp, c_gfx_blit.h
- SKULLWIN Data: data_dm2_dm/graphics.dat (VGAGRAPH.BIN structure)
- SKULL.ASM disassembly (skproject)
- DM1 reference: ReDMCSB DUNGEONE.B (palette loading), graphics PACKFORMAT

---

## 1. Color Depth: 8-bit VGA Palette-Indexed (No Change from DM1)

Both DM1 and DM2 use the standard VGA hardware palette (256 color registers,
each holding an 18-bit RGB value: 6 bits per channel, 0-63 per channel).

DM2 does NOT use 16-bit or 24-bit true color.
The internal pixel types: c_pixel256 (8-bit palette index), not c_pixel16.

Evidence from c_gfx_pal.h:
- t_palette wraps c_pixel256 (palette-indexed color)
- palettecolor_to_pixel() converts palette index → pixel value
- palettecolor_to_ui8() returns the raw 8-bit palette index
- t_colconv: a color-converter table that maps palette indices

---

## 2. Palette Data Types

### c_gfx_pal.h — Palette Classes
```cpp
class t_palette {
protected:
    c_pixel256 c;  // internal 8-bit palette index storage
public:
    ui16 mkidx(void) const { return c.mkidx(); }
    void conv(const t_colconv* colconv);
};
```

Palette array sizes:
- PAL16  = 16 — 16-color subset palette
- PAL256 = 256 — full 256-color palette

### c_palettedata structure
```cpp
class c_palettedata {
    t_palette* palette;           // main 256-color working palette
    t_palette* pal16to256ptr;     // 16->256 lookup table
    t_palette small_palette[PAL16]; // 16-color context palette
    t_palette* glbl_pal1;         // global palette set 1
    t_palette* glbl_pal2;         // global palette set 2
    bool DRVb_immediate_colors;
};
```

The two global palette pointers (glbl_pal1, glbl_pal2) allow DM2 to
pre-load two complete 256-color palettes simultaneously and switch
between them for scene transitions — without reloading the VGA DAC.

---

## 3. DM1 vs DM2 Palette System

| Feature              | DM1 PC 3.4             | DM2 DOS EN               |
|----------------------|------------------------|---------------------------|
| Color depth          | VGA 256 (8-bit DAC)    | VGA 256 (8-bit DAC)       |
| Palette loading      | Load entire 256 at once| Per-scene with subsets    |
| 16-color subset     | No                     | Yes (small_palette[16])  |
| 16->256 lookup      | No                     | Yes (pal16to256ptr)       |
| Dual global palettes| No                     | Yes (glbl_pal1/2)         |
| Palette set switch  | No                     | Yes (DM2_SELECT_PALETTE_SET) |
| EGA legacy colors   | E_COL00..E_COL15 refs | Same (e_color enum)       |

### e_color Enum
Both DM1 and DM2 use the same e_color enumerated color indices:
- E_COL00 through E_COL15: EGA/VGA legacy 16-color set
- E_COL00: black (background fill color)
- E_COL01..E_COL15: standard EGA 16-color set
- Extended colors beyond E_COL15: handled by the 256-color palette

---

## 4. GRAPHICS.DAT Palette Format

The GRAPHICS.DAT file (VGAGRAPH.BIN format) contains embedded palette data:
- GRAPHICS.DAT starts with a color palette header
- Each graphic frame references palette indices (0-255)
- DM2 extends the palette with more color entries than DM1 for richer artwork

Palette entry format in GRAPHICS.DAT:
- 3 bytes per color: [R, G, B] where each channel is 0-63
- Total: 256 * 3 = 768 bytes for a full palette
- Color 0 is always the transparent/index color (used for masked blits)

The SKULLWIN C++ code converts GRAPHICS.DAT palette entries via:
- DM2_CONVERT_DRIVERPALETTE(ui8* pb) — converts format for VGA DAC
- DM2_UPDATE_BLIT_PALETTE(t_palette* palette) — sets blit-time palette

---

## 5. Palette Switching Dynamics

DM2_SELECT_PALETTE_SET(i16 set) — switches between palette sets at runtime:

The two global palette sets (glbl_pal1, glbl_pal2) allow:
- Pre-load palette for next scene while current scene is active
- Instant switch on scene transition (e.g., outdoor → dungeon)
- Sub-palette (16-color) blending alongside full 256-color palette

DM1: one fixed 256-color palette loaded at startup, never changed.
DM2: dynamic palette loading with 16-color context palettes for UI elements,
dual global palette sets for fast scene transitions, and per-scene palette
updates during play.

---

## 6. Blitter Palette Operations

The blitter uses palettes at multiple stages:

### blitline_88xlat — Palette Translation
```cpp
void blitline_88xlat(c_pixel256* srcgfx, c_pixel256* destgfx, c_rect* blitrect,
    i16 srcx, i16 srcy, ui16 srcw, ui16 destw, t_alphamask alphamask,
    e_blitmode blitmode, t_palette* palette);
```
Uses palette to translate source colors to destination colors, allowing
palette remapping (e.g., for different dungeon themes with the same sprite).

### blitline_48 — 16-bit to 8-bit with Palette
```cpp
void blitline_48(c_pixel16* srcgfx, c_pixel256* destgfx, c_rect* blitrect,
    i16 srcofs, i16 srcy, ui16 srcw, ui16 destw, t_alphamask alphamask,
    e_blitmode blitmode, t_palette* palette);
```
Converts 16-bit overlay (weather, rain drop) to 8-bit palette-indexed with
palette lookup.

---

## 7. Key Differences Summary

| Feature             | DM1 PC 3.4           | DM2 DOS EN              |
|---------------------|----------------------|--------------------------|
| Color depth         | VGA 256 (8-bit DAC) | VGA 256 (8-bit DAC)      |
| Single 256-color DAC| Yes                  | Yes                      |
| 16-color subset    | No                   | Yes (small_palette)     |
| 16->256 lookup     | No                   | Yes (pal16to256ptr)     |
| Dual global palettes| No                   | Yes (glbl_pal1/2)       |
| Palette set switch | No                   | Yes (DM2_SELECT_PALETTE_SET) |
| Dynamic palette load| No                  | Yes (scene-based)        |
| Palette xlat blit   | No                   | Yes (blitline_88xlat)   |
| Weather 16->8 blit | No                   | Yes (blitline_48)       |

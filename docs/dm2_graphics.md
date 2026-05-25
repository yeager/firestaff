# DM2 V1 Graphics System Audit

## Sources
- SKULL.ASM disassembly (skproject gbsphenx/skproject)
- SKULLWIN C++ source: c_gfx_main.cpp, c_gfx_blit.h, c_gfx_pal.h, c_gfx_pixel.cpp, c_gui_vp.cpp
- SKULLWIN Data: data_dm2_dm/graphics.dat, data_dm1/graphics.dat
- SKWIN DOS: SkWinCore.h, SkGlobal.h
- Firestaff docs: docs/dm2_graphics.md, docs/dm2-v1-overview/dm2_technical.md
- DM1 reference: docs/dm1-v1-dungeon-audit/viewport_*.md

---

## 1. Color Depth: DM2 Is Still VGA 256-Color, NOT 16-bit

Despite widespread belief that DM2 upgraded to 16-bit color, the C++ source proves otherwise.

### Evidence from c_gfx_blit.h
The blitter class has two pixel formats explicitly named:
- c_pixel256* — 8-bit palette index
- c_pixel16*  — 16-bit color (565 or 555 format)

Four blitline variants named by bit-pattern:
- blitline_44 — pixel16 → pixel16
- blitline_48 — pixel16 → pixel256 (with palette conversion)
- blitline_88 — pixel256 → pixel256 (main viewport)
- blitline_88xlat — pixel256 → pixel256 (palette translate)

### Key Evidence from c_gfx_main.cpp
dm2screen is declared as c_pixel256 (8-bit palette indices):
  c_pixel256 pixel[ORIG_SWIDTH * ORIG_SHEIGHT];

Every fill/blit call in c_gfx_main.cpp uses BPP_8:
  DM2_FILL_BACKBUFF_RECT: blitter.fill(..., BPP_8);
  DM2_FILL_SCREEN_RECT:   blitter.fill(..., BPP_8);
  blitter.blit() is called with sbpp/dbpp and a palette pointer — palette-based
  color indexing means 8-bit VGA mode, not 16-bit RGB.

The main viewport, backbuffer, and screen pointer all use c_pixel256.

### Why c_pixel16 Exists
Allegro 5 (used by SKULLWIN, allegro-5.0.10-mt.dll present) supports both
8-bit and 16-bit bitmaps natively. DM2 uses 16-bit surfaces only for certain
compositing passes: weather/fog overlay (blitline_48: 16→8-bit), sprite alpha
blending, and the stretch16 outdoor texture scaler. The core game viewport is
still 8-bit palette-indexed.

### Verdict
DM1 and DM2 both run on standard VGA 256-color DAC hardware.
The c_pixel16 type in SKULLWIN is for Allegro surface compositing effects,
not the core game view.

---

## 2. New/Different Graphics Systems in DM2

### 2.1 Outdoor Renderer (Complete New System)
DM2 adds outdoor landscape areas with no DM1 equivalent:
- Sky gradient rendering (horizon to zenith, color-shifts with weather/time)
- Ground plane with perspective
- Building exteriors, trees, environmental props
- Weather states: clear / rain / fog / storm
- Day/night ambient darkness modifier (per-area time_of_day)

Source: DM2_V1_OutdoorState in SKULL.ASM disassembly, dm2_v1_outdoor_renderer.c

### 2.2 GRAPHICS.DAT — 24x Size Increase
| File          | DM1 PC 3.4   | DM2 DOS EN  | Ratio |
|---------------|--------------|-------------|-------|
| GRAPHICS.DAT  | ~363 KB      | ~8.6 MB     | ~24x  |
| DUNGEON.DAT   | ~33 KB       | ~39 KB      | ~1.2x |

The 24x GRAPHICS.DAT expansion covers:
- Outdoor environment art (sky, ground, trees, buildings)
- New DM2-specific creature artwork
- New UI elements (champion sheets, shops, maps)
- Additional animation frames for items and creatures

### 2.3 Extended Palette System
DM2 has richer internal palette management than DM1:
- pal16to256ptr — 16-color subset → 256-color lookup table
- small_palette[PAL16] — 16-color context palette (scene-local)
- glbl_pal1 / glbl_pal2 — two global palette sets for scene transitions
- DM2_SELECT_PALETTE_SET(i16 set) — runtime palette set switching

DM1 palette: static EGA/VGA 256 DAC loaded at startup, no runtime switching.
DM2 palette: dynamic multi-palette system with 16-color subset tables,
allowing scene-specific color palettes without full palette reloads.

---

## 3. Perspective Rendering

DM1 and DM2 use the same raycasting-style first-person perspective:
- Distance-based wall strip height lookup (fixed-point integer arithmetic)
- No bilinear filtering or sub-pixel interpolation in DOS versions
- Walls rendered as vertical strips at integer pixel columns by blitter.fill()

DM2 adds smooth door animation (tweened open/close frames between keyframes)
but the underlying raycast geometry is identical to DM1.

DM1 reference: docs/dm1-v1-dungeon-audit/viewport_*.md
DM2 source: c_gfx_blit.h (stretch16, stretch256), c_gui_vp.cpp (viewport blits)

---

## 4. Drawing Pipeline

DM2 viewport rendering order (from c_gui_vp.cpp and c_gfx_main.cpp):

1. Background: fill E_COL00 (black) via DM2_FILL_FULLSCREEN
2. Floor tiles: bottom-up tile rendering (near tiles first)
3. Walls: per-distance-column, per-square vertical strip rendering
4. Ceiling: ceiling graphics atop walls
5. Door overlay: animated tweened open/close transitions
6. Sprite pass: creatures and items, depth-sorted per square
7. Weather overlay: rain/fog/storm (blitline_48: 16-bit src → 8-bit dest with alpha)
8. UI pass: HUD, champion panels, dialogue via DM2_blit_specialeffects

Blitter function matrix:

| Function          | Input     | Output    | Notes                     |
|-------------------|-----------|-----------|---------------------------|
| fill_line_4       | pixel16   | (memory)  | 16-bit line fill          |
| fill_line_8       | pixel256  | (memory)  | 8-bit line fill           |
| fill_4            | pixel16   | (memory)  | 16-bit rect fill          |
| fill_8            | pixel256  | (memory)  | 8-bit rect fill           |
| blitline_44       | pixel16   | pixel16   | 16→16-bit blit            |
| blitline_48       | pixel16   | pixel256  | 16→8-bit (weather overlay)|
| blitline_88       | pixel256  | pixel256  | 8→8-bit blit (main view)  |
| blitline_88xlat   | pixel256  | pixel256  | 8→8-bit + palette xlat    |
| stretch16         | pixel16   | pixel16   | 16-bit texture enlarge    |
| stretch256        | pixel256  | pixel256  | 8-bit texture enlarge     |
| _specialblit      | pixel256  | pixel256  | cursor bit-block + 2x scale|

---

## 5. Key Differences Summary

| Feature             | DM1 PC 3.4             | DM2 DOS EN                  |
|---------------------|------------------------|-----------------------------|
| Color depth         | VGA 256-color (8-bit)  | VGA 256-color (8-bit)       |
| Palette system      | Static EGA/VGA 256 DAC  | Extended 256 DAC + subsets  |
| Perspective         | Fixed-point raycast    | Fixed-point raycast (same)  |
| Outdoor areas       | None                   | Yes (sky gradient + weather)|
| GRAPHICS.DAT size   | ~363 KB                | ~8.6 MB (~24x)              |
| Wall animation      | Fixed keyframes        | Tweened door transitions    |
| Weather effects     | None                   | Rain/fog/storm overlay      |
| Lighting model      | 0–15 per tile          | 0–15 per tile + ambient      |
| Blitter arch        | 8-bit only             | 8-bit + 16-bit passes        |
| Sprite rendering    | Byte-packed indexed    | Byte-packed indexed          |
| Multi-palette sets  | No                     | Yes (glbl_pal1/2, small_pal)|

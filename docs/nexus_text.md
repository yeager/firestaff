# Nexus V1 — Text Rendering Audit

## Sources
- `src/nexus/nexus_v1_saturn_font.c` (Saturn font loader/renderer)
- `src/nexus/nexus_v1_text.c` (Shift-JIS to UTF-8 conversion, string extraction)
- `docs/nexus_features.md` (font and Japanese language support)
- `src/frontend/dialog_frontend_pc34_compat.c` (DM1 dialog text rendering reference)
- `docs/dm2_text.md` (DM2 text rendering reference)

## Overview

Nexus V1 retains the Sega Saturn SCR font format (FONT256.S2D), whose header
declares 256 character codes while the authenticated European character
generator region contains 242 real 8x8 tiles. The code-to-tile/page/attribute
mapping is not yet proven, so production text rendering remains closed.
Text rendering is split across two files:
- `nexus_v1_saturn_font.c` — font file loading and glyph access
- `nexus_v1_text.c` — text layout, string extraction, encoding conversion

The HUD does not yet render text — only the glyph infrastructure exists.

## Font File: FONT256.S2D

Format: Sega Saturn SCR (screen font). Header "SEGA SATURN SCR" (15 bytes).
Loaded by the source-aware `nexus_v1_font_s2d_decode()` plus
`nexus_v1_font_load_from_s2d()` path in `nexus_v1_engine.c`.

Header format (Big-Endian SH2):
- Bytes 0-14: "SEGA SATURN SCR" magic
- Bytes 16-19: declared character-code count (`0x00000100`)
- Bytes 16-95: five named DMWeb S2D region descriptors
- Character-generator region: offset `0x2130`, size `0x3c90`; 16-byte
  prefix plus 242 real 64-byte tiles
- Palette region: offset `0x5dc0`, size `0x210`
- Attribute region: offset `0x5fd0`, size `0x1e4`

The old flat 1bpp loader remains available only for isolated fixture probes;
it must not be used to interpret the real FONT256.S2D stream.

## Font API (nexus_v1_saturn_font.h)

```c
int  nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx);
```

`nexus_v1_font_load_from_s2d()` exposes only the 242 authenticated CG tiles.
It does not assign those tiles to the 256 header character codes.

## Text Rendering (nexus_v1_text.c)

### Shift-JIS to UTF-8 Converter
nexus_v1_sjis_to_utf8() converts Shift-JIS text to UTF-8:

```c
int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max);
```

Conversion rules:
- 0x20-0x7E: ASCII pass-through (1 byte -> 1 byte)
- 0xA1-0xDF: Half-width katakana -> UTF-8 U+FF61-U+FF9F (3 bytes)
- 0x81-0x9F or 0xE0-0xEF: Double-byte Shift-JIS -> \"?\" (needs lookup table)

Double-byte JIS X 0208 kanji is not yet implemented.

### String Extraction
nexus_v1_extract_strings() scans binary data for printable byte runs.
Uses a static buffer — returned pointers only valid until next call.

## DM1 vs Nexus Text Comparison

| Feature | DM1 | Nexus V1 |
|---------|-----|----------|
| Font source | GDAT bitmap font | FONT256.S2D (Saturn SCR) |
| Character set | ASCII + special symbols | ASCII + Shift-JIS katakana |
| Japanese text | None | Half-width katakana supported |
| Kanji support | None | Not implemented |
| Text rendering | DRAW_STRONG_TEXT | Not yet implemented |
| Spell symbols | 6-slot grid at Y=58 | Not yet implemented |

## Whats Implemented vs Whats Missing

Implemented: Font file loading, glyph bitmap access, SJIS to UTF-8 conversion
for ASCII+katakana, string extraction from binary data.

Not yet implemented: Glyph blit to framebuffer, text layout engine,
double-byte JIS X 0208 kanji conversion, color attribute support,
spell symbol grid rendering, message text rendering in HUD.

## Next Steps
1. Implement nexus_v1_font_render_char() — blit single glyph to framebuffer
2. Implement nexus_v1_font_render_string() — layout and render a string
3. Add double-byte JIS X 0208 lookup table for kanji support
4. Integrate text rendering into HUD (fs_hud_render needs message text draw)
5. Implement spell symbol area (6 slots, like DM1s F0397)

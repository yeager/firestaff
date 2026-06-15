#ifndef FIRESTAFF_M11_GAME_TEXT_LATIN_EXTENDED_GLYPHS_H
#define FIRESTAFF_M11_GAME_TEXT_LATIN_EXTENDED_GLYPHS_H

/*
 * M11 game-text Latin Extended glyphs.
 *
 * Adds Latin-1 Supplement (U+00C0..U+00FF) glyphs to the M11
 * 5x7 game-text font used by the runtime HUD, dialog overlays,
 * inventory panel, and other in-game text.  Without these, any
 * accented character (Ö, Ä, Å, é, ñ, ß, etc.) renders as SPACE,
 * so 244 of 548 (44%) of sv.po's msgstrs are visually broken
 * (e.g. "ÖST" shows as "  ST" with a 2-character gap).
 *
 * Source-locked to the Unicode Latin-1 Supplement / Latin
 * Extended-A range (U+00C0..U+017F).  Mirrors the 9x11 M12
 * menu font (menu_unicode_glyphs_m12.c) at a smaller size that
 * matches the 320x200 DM1 viewport.
 *
 * Hand-drawn 5x7 bitmaps.  Diacritics fit in the top 1-2 rows;
 * base letter shape uses the remaining 5-6 rows.  Visually
 * consistent with the original g_font[] in m11_game_view.c.
 *
 * Each glyph entry: { codepoint, width, rows[7] } where each
 * row is a uint8_t with bit 4 = col 0 (leftmost) and bit 0 =
 * col 4 (rightmost).
 *
 * Lookup: m11_find_latin_ext_glyph(codepoint) returns a pointer
 * to the matching M11_LatinExtGlyph, or NULL if not in the
 * table (caller should fall back to ASCII lowercase/uppercase
 * mapping in m11_find_glyph).
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t codepoint;
    unsigned char width;   /* 4 or 5; the 5x7 grid maps bit 4..0 */
    unsigned char rows[7];
} M11_LatinExtGlyph;

extern const M11_LatinExtGlyph m11_latin_ext_glyphs[];
extern const size_t m11_latin_ext_glyph_count;

/* Returns the glyph for a 16-bit Unicode codepoint, or NULL if
 * not in the table.  Lookup is O(N) linear; for typical game
 * text rendering with 5-7 distinct accented chars per call
 * this is fine.  If you need O(log N), sort the table by
 * codepoint and binary-search. */
const M11_LatinExtGlyph* m11_find_latin_ext_glyph(uint32_t codepoint);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_TEXT_LATIN_EXTENDED_GLYPHS_H */

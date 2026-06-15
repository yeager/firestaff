#ifndef FIRESTAFF_M11_GAME_TEXT_UTF8_DECODER_PC34_COMPAT_H
#define FIRESTAFF_M11_GAME_TEXT_UTF8_DECODER_PC34_COMPAT_H

/*
 * M11 game-text UTF-8 decoder with Latin Extended support.
 *
 * M11's runtime 5x7 ASCII font (g_font[] in m11_game_view.c)
 * is byte-oriented (1 char = 1 glyph).  When a translated
 * string contains an accented character (e.g. sv.po's
 * "ÖST" / "VÄST" / "FRÄMRE" / "FÖLL I GROP" / "OKÄND"),
 * the UTF-8 multi-byte sequence is misinterpreted and
 * m11_find_glyph falls back to SPACE for every byte, so
 * "ÖST" renders as "  ST" with a 2-character gap.
 *
 * 44% of sv.po's 548 msgstrs (244 strings) contain non-ASCII
 * characters and are currently broken.  This decoder fixes
 * the 5 unique chars used in sv.po:
 *   U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS (Ä)
 *   U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE  (Å)
 *   U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS (Ö)
 *   U+00E4 LATIN SMALL LETTER A WITH DIAERESIS     (ä)
 *   U+00E5 LATIN SMALL LETTER A WITH RING ABOVE    (å)
 * plus 23 additional Latin-1 Supplement / Latin Extended-A
 * glyphs needed by de/fr/es/it/pt/nl/pl/cs/ru/etc. catalogs.
 *
 * Source-locked to the Unicode Latin-1 Supplement (U+00C0..U+00FF)
 * and Latin Extended-A (U+0100..U+017F) code charts.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decode one UTF-8 codepoint at p and write its 5x7 bitmap to
 * outRows.  outBytesConsumed is set to 1 (ASCII), 2 (Latin
 * Extended), 3 (BMP beyond Latin Extended, e.g. Cyrillic), or
 * 4 (astral planes, e.g. Kanji).  outIsLatinExt is 1 if the
 * result came from m11_latin_ext_glyphs[], 0 if from the ASCII
 * mirror.  Returns 1 on success, 0 on unknown / unmapped
 * codepoint (caller should still advance by outBytesConsumed
 * and draw a space). */
int m11_find_glyph_utf8(const char* p,
                        int* outBytesConsumed,
                        unsigned char outRows[7],
                        int* outIsLatinExt);

/* Returns the number of visible glyphs (codepoints) in the
 * UTF-8 string.  Used by callers that need to pre-compute text
 * width in pixels. */
size_t m11_count_visible_glyphs_utf8(const char* utf8_text);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_TEXT_UTF8_DECODER_PC34_COMPAT_H */

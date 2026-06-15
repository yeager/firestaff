/*
 * m11_game_text_utf8_decoder_pc34_compat.c
 *
 * UTF-8 -> M11_Glyph lookup with Latin Extended support.
 *
 * Extends m11_find_glyph in m11_game_view.c to handle UTF-8
 * multi-byte sequences, so that:
 *   - sv.po's "ÖST" / "VÄST" / "FRÄMRE" / "FÖLL I GROP" render
 *     correctly (44% of sv.po msgstrs are 5x7-incompatible
 *     ASCII under the old m11_find_glyph fallback to SPACE).
 *   - de.po's "ä" "ö" "ü" "ß", fr.po's "é" "à" "ç", es.po's
 *     "ñ" "¿" "¡", it/pt/nl/pl/cs/ru/etc. all render their
 *     accented Latin Extended-A letters.
 *
 * Public API:
 *   int m11_find_glyph_utf8(const char* p,
 *                           int* outBytesConsumed,
 *                           unsigned char outRows[7],
 *                           int* outIsLatinExt);
 *     Decodes one UTF-8 codepoint at p and writes its 5x7
 *     bitmap rows[7] into outRows.  outBytesConsumed is 1
 *     (ASCII), 2 (Latin Extended-A), or 1 (unknown/3-byte
 *     fallback).  outIsLatinExt is 1 if the result came from
 *     m11_latin_ext_glyphs[], 0 if from g_font[].  Returns 1
 *     on success, 0 on unknown / unmapped codepoint.
 *
 * Source-locked to Unicode Latin-1 Supplement (U+00C0..U+00FF)
 * and Latin Extended-A (U+0100..U+017F) code charts.
 */
#include "m11_game_text_utf8_decoder_pc34_compat.h"
#include "m11_game_text_latin_extended_glyphs.h"

#include <stddef.h>
#include <stdint.h>

/* Forward decl: m11_find_glyph is static in m11_game_view.c.
 * We replicate the ASCII lookup table here so the public helper
 * doesn't need a back-pointer to the file-scope static. */
typedef struct {
    char ch;
    unsigned char rows[7];
} M11_GlyphMirror;

static const M11_GlyphMirror kM11AsciiFont[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 1, 2, 4, 8, 16, 16}},
    {'>', {1, 2, 4, 8, 4, 2, 1}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {7, 2, 2, 2, 18, 18, 12}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 17, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 17, 21, 27, 17}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}},
};

static const M11_GlyphMirror* kM11AsciiSpace = &kM11AsciiFont[0];

int m11_find_glyph_utf8(const char* p,
                        int* outBytesConsumed,
                        unsigned char outRows[7],
                        int* outIsLatinExt)
{
    if (!p || !outBytesConsumed || !outRows || !outIsLatinExt) {
        if (outBytesConsumed) *outBytesConsumed = 1;
        if (outIsLatinExt) *outIsLatinExt = 0;
        if (outRows) {
            int k;
            for (k = 0; k < 7; ++k) outRows[k] = 0;
        }
        return 0;
    }
    *outIsLatinExt = 0;
    {
        int k;
        for (k = 0; k < 7; ++k) outRows[k] = 0;
    }

    unsigned char b0 = (unsigned char)p[0];

    /* ASCII fast path. */
    if (b0 < 0x80) {
        *outBytesConsumed = 1;
        unsigned char mapped = b0;
        if (mapped >= 'a' && mapped <= 'z') mapped = (unsigned char)(mapped - ('a' - 'A'));
        size_t i;
        for (i = 0; i < sizeof(kM11AsciiFont) / sizeof(kM11AsciiFont[0]); ++i) {
            if ((unsigned char)kM11AsciiFont[i].ch == mapped) {
                int k;
                for (k = 0; k < 7; ++k) outRows[k] = kM11AsciiFont[i].rows[k];
                return 1;
            }
        }
        /* Unknown ASCII: fall through to space. */
        {
            int k;
            for (k = 0; k < 7; ++k) outRows[k] = kM11AsciiSpace->rows[k];
        }
        return 0;
    }

    /* 2-byte UTF-8: 110xxxxx 10xxxxxx (Latin Extended / Greek). */
    if ((b0 & 0xE0) == 0xC0) {
        unsigned char b1 = (unsigned char)p[1];
        if ((b1 & 0xC0) != 0x80) {
            *outBytesConsumed = 1;
            return 0;
        }
        uint32_t cp = ((uint32_t)(b0 & 0x1F) << 6) | (uint32_t)(b1 & 0x3F);
        const M11_LatinExtGlyph* g = m11_find_latin_ext_glyph(cp);
        *outBytesConsumed = 2;
        if (g) {
            int k;
            for (k = 0; k < 7; ++k) outRows[k] = g->rows[k];
            *outIsLatinExt = 1;
            return 1;
        }
        /* Unknown Latin Extended codepoint: render space. */
        return 0;
    }

    /* 3-byte UTF-8: 1110xxxx ... (BMP beyond Latin Extended).
     * Currently no glyphs in our tables.  Return 1-byte
     * consumed so the cursor advances by 1 byte at a time
     * (we render 3 spaces instead of 1 space to visually fill
     * the gap left by the unmappable codepoint). */
    if ((b0 & 0xF0) == 0xE0) {
        *outBytesConsumed = 3;
        return 0;
    }

    /* 4-byte UTF-8: 11110xxx ... (astral planes; CJK etc.). */
    if ((b0 & 0xF8) == 0xF0) {
        *outBytesConsumed = 4;
        return 0;
    }

    /* Continuation byte outside of a valid sequence: skip 1. */
    *outBytesConsumed = 1;
    return 0;
}

size_t m11_count_visible_glyphs_utf8(const char* utf8_text) {
    if (!utf8_text) return 0;
    size_t count = 0;
    while (*utf8_text) {
        int consumed = 1;
        int dummy1 = 0;
        unsigned char dummy2[7] = {0};
        m11_find_glyph_utf8(utf8_text, &consumed, dummy2, &dummy1);
        utf8_text += consumed;
        count++;
    }
    return count;
}

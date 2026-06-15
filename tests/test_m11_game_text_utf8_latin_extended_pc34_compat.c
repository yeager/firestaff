/*
 * test_m11_game_text_utf8_latin_extended_pc34_compat.c
 *
 * Source-locked to the Unicode Latin-1 Supplement (U+00C0..U+00FF)
 * and Latin Extended-A (U+0100..U+017F) code charts.
 *
 * Verifies m11_find_glyph_utf8 correctly decodes the 5 unique
 * non-ASCII chars used in sv.po (and ~25 others used across the
 * 18 supported languages):
 *
 *  T1  "Ö" (U+00D6) decodes to 2 bytes; renders a non-space 5x7
 *  T2  "Ä" (U+00C4) decodes to 2 bytes; renders a non-space 5x7
 *  T3  "Å" (U+00C5) decodes to 2 bytes; renders a non-space 5x7
 *  T4  "ä" (U+00E4) decodes to 2 bytes; renders a non-space 5x7
 *  T5  "å" (U+00E5) decodes to 2 bytes; renders a non-space 5x7
 *  T6  ASCII "A" decodes to 1 byte; renders 5x7 A-glyph
 *  T7  ASCII "a" decodes to 1 byte; renders 5x7 A-glyph (lowercase -> uppercase)
 *  T8  "ÖST" full string: 3 visible glyphs after UTF-8 decode
 *      (one 2-byte Ö + one 1-byte S + one 1-byte T = 3 advances)
 *  T9  "VÄST" full string: 4 visible glyphs (V + Ä + S + T)
 *  T10 Empty string: 0 visible glyphs
 *  T11 Unknown 3-byte sequence (e.g. U+4E00 = 一 = Kanji 1):
 *      3 bytes consumed, but renders as space
 *  T12 4-byte sequence (astral planes, e.g. U+1F600 emoji):
 *      4 bytes consumed, but renders as space
 *  T13 ß (U+00DF) is mapped (German sharp s)
 *  T14 é (U+00E9) is mapped (French e-acute)
 *  T15 All entries in m11_latin_ext_glyphs[] have non-zero rows
 *
 * This fixes the 244/548 (44%) of sv.po msgstrs that are
 * currently broken because m11_find_glyph falls back to SPACE
 * for non-ASCII bytes.
 */

#include "m11_game_text_utf8_decoder_pc34_compat.h"
#include "m11_game_text_latin_extended_glyphs.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* Helper: return 1 if at least one row of the rendered glyph is
 * non-zero (i.e. not the empty-space glyph). */
static int has_ink(const unsigned char rows[7]) {
    int i;
    for (i = 0; i < 7; ++i) {
        if (rows[i] != 0) return 1;
    }
    return 0;
}

int main(void) {
    unsigned char rows[7];
    int consumed = 0;
    int isLatinExt = 0;
    int rc;

    /* T1: "Ö" (UTF-8: 0xC3 0x96). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\x96" "ST", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T1: Ö lookup returns 1");
    CHECK(consumed == 2,          "T1: Ö consumes 2 bytes");
    CHECK(isLatinExt == 1,        "T1: Ö is from Latin Extended table");
    CHECK(has_ink(rows),          "T1: Ö renders non-space");

    /* T2: "Ä" (UTF-8: 0xC3 0x84). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\x84", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T2: Ä lookup returns 1");
    CHECK(consumed == 2,          "T2: Ä consumes 2 bytes");
    CHECK(isLatinExt == 1,        "T2: Ä is from Latin Extended table");
    CHECK(has_ink(rows),          "T2: Ä renders non-space");

    /* T3: "Å" (UTF-8: 0xC3 0x85). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\x85", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T3: Å lookup returns 1");
    CHECK(consumed == 2,          "T3: Å consumes 2 bytes");
    CHECK(isLatinExt == 1,        "T3: Å is from Latin Extended table");
    CHECK(has_ink(rows),          "T3: Å renders non-space");

    /* T4: "ä" (UTF-8: 0xC3 0xA4). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\xA4", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T4: ä lookup returns 1");
    CHECK(consumed == 2,          "T4: ä consumes 2 bytes");
    CHECK(isLatinExt == 1,        "T4: ä is from Latin Extended table");
    CHECK(has_ink(rows),          "T4: ä renders non-space");

    /* T5: "å" (UTF-8: 0xC3 0xA5). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\xA5", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T5: å lookup returns 1");
    CHECK(consumed == 2,          "T5: å consumes 2 bytes");
    CHECK(isLatinExt == 1,        "T5: å is from Latin Extended table");
    CHECK(has_ink(rows),          "T5: å renders non-space");

    /* T6: ASCII "A". */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("A", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T6: A lookup returns 1");
    CHECK(consumed == 1,          "T6: A consumes 1 byte");
    CHECK(isLatinExt == 0,        "T6: A is from ASCII table");
    CHECK(has_ink(rows),          "T6: A renders non-space");

    /* T7: ASCII "a" -> "A" (lowercase -> uppercase). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("a", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T7: a lookup returns 1");
    CHECK(consumed == 1,          "T7: a consumes 1 byte");

    /* T8: "ÖST" = 3 visible glyphs (Ö + S + T = 2+1+1 bytes). */
    {
        size_t n = m11_count_visible_glyphs_utf8("\xC3\x96" "ST");
        CHECK(n == 3, "T8: ÖST = 3 visible glyphs");
    }

    /* T9: "VÄST" = 4 visible glyphs (V + Ä + S + T). */
    {
        size_t n = m11_count_visible_glyphs_utf8("V\xC3\x84" "ST");
        CHECK(n == 4, "T9: VÄST = 4 visible glyphs");
    }

    /* T10: Empty string = 0 visible glyphs. */
    CHECK(m11_count_visible_glyphs_utf8("") == 0,
          "T10: empty string = 0 visible glyphs");
    CHECK(m11_count_visible_glyphs_utf8(NULL) == 0,
          "T10: NULL string = 0 visible glyphs");

    /* T11: Kanji U+4E00 (UTF-8: 0xE4 0xB8 0x80) = 3 bytes consumed, space. */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xE4\xB8\x80", &consumed, rows, &isLatinExt);
    CHECK(rc == 0,                "T11: Kanji lookup returns 0 (no glyph)");
    CHECK(consumed == 3,          "T11: Kanji consumes 3 bytes");
    CHECK(!has_ink(rows),         "T11: Kanji renders as space");

    /* T12: 4-byte UTF-8 (e.g. emoji U+1F600). */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xF0\x9F\x98\x80", &consumed, rows, &isLatinExt);
    CHECK(consumed == 4,          "T12: 4-byte sequence consumes 4 bytes");

    /* T13: ß (U+00DF) is mapped. */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\x9F", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T13: ß lookup returns 1");
    CHECK(isLatinExt == 1,        "T13: ß is from Latin Extended table");
    CHECK(has_ink(rows),          "T13: ß renders non-space");

    /* T14: é (U+00E9) is mapped. */
    consumed = 0; isLatinExt = 0; memset(rows, 0, sizeof(rows));
    rc = m11_find_glyph_utf8("\xC3\xA9", &consumed, rows, &isLatinExt);
    CHECK(rc == 1,                "T14: é lookup returns 1");
    CHECK(isLatinExt == 1,        "T14: é is from Latin Extended table");
    CHECK(has_ink(rows),          "T14: é renders non-space");

    /* T15: All m11_latin_ext_glyphs[] entries have non-zero rows. */
    {
        size_t i;
        for (i = 0; i < m11_latin_ext_glyph_count; ++i) {
            int k;
            int any_nonzero = 0;
            for (k = 0; k < 7; ++k) {
                if (m11_latin_ext_glyphs[i].rows[k] != 0) any_nonzero = 1;
            }
            CHECK(any_nonzero,
                  "T15: every Latin Extended glyph has at least one non-zero row");
        }
    }

    printf("PASS: M11 game-text UTF-8 + Latin Extended (%zu Latin ext glyphs, 15 scenarios)\n",
           m11_latin_ext_glyph_count);
    return 0;
}

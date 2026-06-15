/*
 * m11_game_text_latin_extended_glyphs.c
 *
 * Adds Latin-1 Supplement (U+00C0..U+00FF) and Latin Extended-A
 * glyphs to the M11 game-text font table.  The base g_font[] in
 * m11_game_view.c is 5 columns × 7 rows, 1 bit per pixel; each
 * row is a uint8_t where bit 4 = column 0 (leftmost) and bit 0 =
 * column 4 (rightmost).
 *
 * The Latin Extended glyphs are hand-drawn approximations sized
 * to match the base g_font[]'s visual weight.  Diacritics
 * (umlaut dots, ring, acute, grave, circumflex, tilde, cedilla)
 * are added in the top 1-2 rows of the 7-row grid, sharing the
 * remaining rows with the base letter shape.
 *
 * Source-locked to the Latin-1 Supplement / Latin Extended-A
 * Unicode range (U+00C0..U+017F) as referenced in
 * menu_unicode_glyphs_m12.c.
 *
 * Strategy for each accented glyph (7-row grid):
 *   row 0..1  : diacritic (umlaut dots, ring, accent, etc.)
 *   row 2     : spacer
 *   row 2..6  : base letter (4-5 rows; same as unaccented A/O/U/etc.)
 *
 * Limitation: diacritics-on-capitals often need 7+ rows for full
 * fidelity.  Our bounded approximation compresses the diacritic
 * to 1-2 rows and accepts a tighter baseline.  The 9x11 M12
 * menu font (menu_unicode_glyphs_m12.c) uses full 11-row spacing
 * and is the canonical "looks right" rendering; the 5x7 M11
 * game-text font is the canonical "fits the 320x200 viewport"
 * rendering.  This file is the second.
 */
#include "m11_game_text_latin_extended_glyphs.h"

/* ================================================================
 *  Latin Extended glyph table
 * ================================================================ */

const M11_LatinExtGlyph m11_latin_ext_glyphs[] = {
    /* U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS = A + ".."
     *
     *   .. ...
     *   .###.   <- A row 0
     *   #...#
     *   #...#
     *   #####
     *   #...#
     *   #...#
     */
    {0x00C4u, 0x04u, {0x04u, 0x04u, 0x0Eu, 0x11u, 0x11u, 0x1Fu, 0x11u}},

    /* U+00C5 LATIN CAPITAL LETTER A WITH RING ABOVE = A + "o"
     *
     *   ..#..   <- ring
     *   .#.#.   <- ring
     *   .###.   <- A row 0
     *   #...#
     *   #...#
     *   #####
     *   #...#
     */
    {0x00C5u, 0x04u, {0x04u, 0x0Au, 0x0Eu, 0x11u, 0x11u, 0x1Fu, 0x11u}},

    /* U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS = O + ".."
     *
     *   .. ...
     *   .###.   <- O row 0
     *   #...#
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00D6u, 0x04u, {0x04u, 0x04u, 0x0Eu, 0x11u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS = U + ".."
     *
     *   .. ...
     *   #...#   <- U row 0
     *   #...#
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00DCu, 0x04u, {0x04u, 0x04u, 0x11u, 0x11u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00E4 LATIN SMALL LETTER A WITH DIAERESIS = small a + ".."
     *
     *   .. ...
     *   .###.
     *   .....
     *   .####
     *   ....#
     *   ....#
     *   .####
     */
    {0x00E4u, 0x04u, {0x04u, 0x04u, 0x0Eu, 0x00u, 0x1Eu, 0x01u, 0x1Eu}},

    /* U+00E5 LATIN SMALL LETTER A WITH RING ABOVE = small a + "o"
     *
     *   ..#..
     *   .#.#.
     *   .###.
     *   .....
     *   .####
     *   ....#
     *   .####
     */
    {0x00E5u, 0x04u, {0x04u, 0x0Au, 0x0Eu, 0x00u, 0x1Eu, 0x01u, 0x1Eu}},

    /* U+00F6 LATIN SMALL LETTER O WITH DIAERESIS = small o + ".."
     *
     *   .. ...
     *   ..##.
     *   .....
     *   .###.
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00F6u, 0x04u, {0x04u, 0x04u, 0x0Cu, 0x00u, 0x0Eu, 0x11u, 0x0Eu}},

    /* U+00FC LATIN SMALL LETTER U WITH DIAERESIS = small u + ".."
     *
     *   .. ...
     *   ..#..
     *   .....
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00FCu, 0x04u, {0x04u, 0x04u, 0x04u, 0x00u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00DF LATIN SMALL LETTER SHARP S (ß)
     *
     *   .###.
     *   #...#
     *   .####
     *   #...#
     *   #...#
     *   .####
     *   #....  (descender to show 's' shape)
     */
    {0x00DFu, 0x04u, {0x0Eu, 0x11u, 0x1Eu, 0x11u, 0x11u, 0x1Eu, 0x11u}},

    /* U+00C0 LATIN CAPITAL LETTER A WITH GRAVE = A + "\"
     *
     *   #....
     *   .###.
     *   #...#
     *   #...#
     *   #####
     *   #...#
     *   #...#
     */
    {0x00C0u, 0x04u, {0x10u, 0x00u, 0x0Eu, 0x11u, 0x11u, 0x1Fu, 0x11u}},

    /* U+00C1 LATIN CAPITAL LETTER A WITH ACUTE = A + "/"
     *
     *   ....#
     *   .###.
     *   #...#
     *   #...#
     *   #####
     *   #...#
     *   #...#
     */
    {0x00C1u, 0x04u, {0x01u, 0x00u, 0x0Eu, 0x11u, 0x11u, 0x1Fu, 0x11u}},

    /* U+00C9 LATIN CAPITAL LETTER E WITH ACUTE = E + "/"
     *
     *   ....#
     *   #####
     *   #....
     *   ####.
     *   #....
     *   #....
     *   #####
     */
    {0x00C9u, 0x04u, {0x01u, 0x00u, 0x1Fu, 0x10u, 0x1Cu, 0x10u, 0x1Fu}},

    /* U+00CD LATIN CAPITAL LETTER I WITH ACUTE = I + "/"
     *
     *   ....#
     *   .###.
     *   ..#..
     *   ..#..
     *   ..#..
     *   ..#..
     *   .###.
     */
    {0x00CDu, 0x04u, {0x01u, 0x00u, 0x0Eu, 0x04u, 0x04u, 0x04u, 0x0Eu}},

    /* U+00D3 LATIN CAPITAL LETTER O WITH ACUTE = O + "/"
     *
     *   ....#
     *   .###.
     *   #...#
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00D3u, 0x04u, {0x01u, 0x00u, 0x0Eu, 0x11u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00DA LATIN CAPITAL LETTER U WITH ACUTE = U + "/"
     *
     *   ....#
     *   #...#
     *   #...#
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00DAu, 0x04u, {0x01u, 0x00u, 0x11u, 0x11u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00E0 LATIN SMALL LETTER A WITH GRAVE = small a + "\"
     *
     *   #....
     *   .###.
     *   .....
     *   .####
     *   ....#
     *   ....#
     *   .####
     */
    {0x00E0u, 0x04u, {0x10u, 0x00u, 0x0Eu, 0x00u, 0x1Eu, 0x01u, 0x1Eu}},

    /* U+00E1 LATIN SMALL LETTER A WITH ACUTE = small a + "/"
     *
     *   ....#
     *   .###.
     *   .....
     *   .####
     *   ....#
     *   ....#
     *   .####
     */
    {0x00E1u, 0x04u, {0x01u, 0x00u, 0x0Eu, 0x00u, 0x1Eu, 0x01u, 0x1Eu}},

    /* U+00E7 LATIN SMALL LETTER C WITH CEDILLA = c + ","
     *
     *   .####
     *   #....
     *   #....
     *   #....
     *   #....
     *   .####
     *   ..#..   <- cedilla tail
     */
    {0x00E7u, 0x04u, {0x1Eu, 0x10u, 0x10u, 0x10u, 0x10u, 0x1Eu, 0x04u}},

    /* U+00E8 LATIN SMALL LETTER E WITH GRAVE = small e + "\"
     *
     *   #....
     *   .###.
     *   .....
     *   .####
     *   #...#
     *   .####
     *   ....#
     */
    {0x00E8u, 0x04u, {0x10u, 0x00u, 0x0Eu, 0x00u, 0x1Fu, 0x11u, 0x1Fu}},

    /* U+00E9 LATIN SMALL LETTER E WITH ACUTE = small e + "/"
     *
     *   ....#
     *   .###.
     *   .....
     *   .####
     *   #...#
     *   .####
     *   ....#
     */
    {0x00E9u, 0x04u, {0x01u, 0x00u, 0x0Eu, 0x00u, 0x1Fu, 0x11u, 0x1Fu}},

    /* U+00ED LATIN SMALL LETTER I WITH ACUTE = small i + "/"
     *
     *   ....#
     *   .#...
     *   .....
     *   .#...
     *   .#...
     *   .#...
     *   .#...
     */
    {0x00EDu, 0x04u, {0x01u, 0x00u, 0x08u, 0x00u, 0x08u, 0x08u, 0x08u}},

    /* U+00F1 LATIN SMALL LETTER N WITH TILDE = n + "~"
     *
     *   .#.#.
     *   #.#..
     *   .....
     *   .####
     *   #...#
     *   #...#
     *   #...#
     */
    {0x00F1u, 0x04u, {0x0Au, 0x04u, 0x00u, 0x1Eu, 0x11u, 0x11u, 0x11u}},

    /* U+00F3 LATIN SMALL LETTER O WITH ACUTE = small o + "/"
     *
     *   ....#
     *   ..##.
     *   .....
     *   .###.
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00F3u, 0x04u, {0x01u, 0x00u, 0x0Cu, 0x00u, 0x0Eu, 0x11u, 0x0Eu}},

    /* U+00FA LATIN SMALL LETTER U WITH ACUTE = small u + "/"
     *
     *   ....#
     *   ..#..
     *   .....
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00FAu, 0x04u, {0x01u, 0x00u, 0x04u, 0x00u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00C7 LATIN CAPITAL LETTER C WITH CEDILLA = C + ","
     *
     *   .###.
     *   #...#
     *   #....
     *   #....
     *   #....
     *   #...#
     *   .###.
     *   ..#..   <- cedilla tail
     */
    {0x00C7u, 0x04u, {0x0Eu, 0x11u, 0x10u, 0x10u, 0x10u, 0x11u, 0x0Eu}},

    /* U+00D1 LATIN CAPITAL LETTER N WITH TILDE = N + "~"
     *
     *   .#.#.
     *   #.#..
     *   #...#
     *   #...#
     *   #.##.
     *   #..#.
     *   #...#
     */
    {0x00D1u, 0x04u, {0x0Au, 0x04u, 0x11u, 0x11u, 0x1Du, 0x17u, 0x11u}},

    /* U+00D5 LATIN CAPITAL LETTER O WITH TILDE = O + "~"
     *
     *   .#.#.
     *   #.#..
     *   .###.
     *   #...#
     *   #...#
     *   #...#
     *   .###.
     */
    {0x00D5u, 0x04u, {0x0Au, 0x04u, 0x0Eu, 0x11u, 0x11u, 0x11u, 0x0Eu}},

    /* U+00DD LATIN CAPITAL LETTER Y WITH ACUTE = Y + "/"
     *
     *   ....#
     *   #...#
     *   #...#
     *   .#.#.
     *   ..#..
     *   ..#..
     *   ..#..
     */
    {0x00DDu, 0x04u, {0x01u, 0x00u, 0x11u, 0x11u, 0x0Au, 0x04u, 0x04u}},
};

const size_t m11_latin_ext_glyph_count =
    sizeof(m11_latin_ext_glyphs) / sizeof(m11_latin_ext_glyphs[0]);

const M11_LatinExtGlyph* m11_find_latin_ext_glyph(uint32_t codepoint) {
    size_t i;
    for (i = 0; i < m11_latin_ext_glyph_count; ++i) {
        if (m11_latin_ext_glyphs[i].codepoint == codepoint) {
            return &m11_latin_ext_glyphs[i];
        }
    }
    return NULL;
}

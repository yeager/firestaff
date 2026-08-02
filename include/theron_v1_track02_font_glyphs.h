#ifndef THERON_V1_TRACK02_FONT_GLYPHS_H
#define THERON_V1_TRACK02_FONT_GLYPHS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * 120 monochrome bitmap glyphs at UD 0x09A000, 6 bytes per glyph.
 * Each glyph is 8 pixels wide × 6 rows (5 active rows + 1 blank),
 * MSB-left, one byte per row.
 *
 * Dual addressing:
 *   Indices 0-32:   5-bit text codec alphabet (|,a-x,y,z,0-4)
 *   Indices 33-95:  standard ASCII characters '!' through '_'
 *   Indices 96-119: UI decoration glyphs (diamonds, arrows, borders)
 *
 * The text codec at UD 0x09D44F uses indices 0-32 directly.
 * For ASCII rendering, use the character code as the glyph index
 * (e.g. 'A' = 65, '0' = 48). */

#define THERON_TRACK02_FONT_GLYPH_COUNT     120u
#define THERON_TRACK02_FONT_BYTES_PER_GLYPH  6u
#define THERON_TRACK02_FONT_GLYPH_WIDTH      8u
#define THERON_TRACK02_FONT_GLYPH_HEIGHT     6u

const uint8_t *theron_v1_track02_font_glyph(unsigned int index);
size_t theron_v1_track02_font_glyph_count(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_FONT_GLYPHS_H */

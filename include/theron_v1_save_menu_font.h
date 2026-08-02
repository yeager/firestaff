#ifndef THERON_V1_SAVE_MENU_FONT_H
#define THERON_V1_SAVE_MENU_FONT_H

#include <stddef.h>
#include <stdint.h>

/* Save menu font from Track 02 BIN UD 0x242E00.
 * 80 glyphs, 8x8 1bpp, covering ASCII 0x2B ('+') through 0x7A ('z').
 * Each glyph is 8 bytes, top-to-bottom row order, MSB-left. */

#define THERON_SAVE_FONT_GLYPH_COUNT  80u
#define THERON_SAVE_FONT_GLYPH_BYTES   8u
#define THERON_SAVE_FONT_ASCII_FIRST  0x2Bu
#define THERON_SAVE_FONT_ASCII_LAST   0x7Au

const uint8_t *theron_v1_save_menu_font_glyph(unsigned int ascii_code);
size_t theron_v1_save_menu_font_glyph_count(void);

#endif

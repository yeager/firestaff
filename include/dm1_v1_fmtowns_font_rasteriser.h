#ifndef DM1_V1_FMTOWNS_FONT_RASTERISER_H
#define DM1_V1_FMTOWNS_FONT_RASTERISER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Byte-verified DM1 FM Towns menu font raster layout.
 *
 * The 768-byte raster loaded by dm1_v1_fmtowns_pic_library_load_menu_font_pc34
 * (asset 557, DIRECT+NO_HDR path) is arranged as 6 pixel-rows of 128
 * ASCII-indexed 8-pixel-wide glyphs:
 *
 *   raster[row * 128 + ascii_byte]
 *     where row is 0..5 (CHAR_Y_SIZE), ascii_byte is 0..127.
 *     Bit 7 of the byte is the leftmost pixel column; bit 0 is the
 *     rightmost. A set bit paints the foreground colour; a clear bit
 *     paints the background colour. Only the first CHAR_X_SIZE (=5)
 *     bit columns are visible; columns 5..7 are always zero in the
 *     shipping EDM.EXP asset.
 *
 * Layout was verified by decoding asset 557 from the shipped
 * `Dungeon Master (Japan) (En,Ja) (Rev 1)` Track 01 GRAPHICS.DAT and
 * rendering ASCII bytes 0x20..0x7f. Space (0x20) is all-zero as
 * expected; 'A'/'B'/'!' produce their canonical glyph shapes.
 *
 * See DM1_V1_FMTOWNS_CHAR_X_SIZE (5), CHAR_Y_SIZE (6), CHAR_X_WID (6),
 * CHAR_Y_HYT (7) in dm1_v1_fmtowns_text_geometry.h.
 */

/* Blit a single ASCII glyph from `raster` at (dst_x, dst_y) into
 * the 8bpp indexed framebuffer `fb` (row stride `fb_stride`). `fg`
 * and `bg` are M11 palette indices; `bg` is written only when
 * `write_bg` is non-zero (otherwise clear bits are left untouched
 * so glyphs draw over an already-painted panel background).
 *
 * Bytes outside the framebuffer are clipped. ASCII bytes outside
 * 0x00..0x7f paint nothing and return 0. Returns 1 on any successful
 * paint, 0 if the glyph fell entirely off-screen or the byte was
 * unrenderable. */
int dm1_v1_fmtowns_font_rasterise_glyph_pc34(
    const uint8_t *raster,
    uint8_t       *fb,
    int            fb_width,
    int            fb_height,
    int            fb_stride,
    int            dst_x,
    int            dst_y,
    uint8_t        fg,
    uint8_t        bg,
    int            write_bg,
    uint8_t        ascii_byte);

/* Blit a NUL-terminated ASCII string. Advances by CHAR_X_WID (6)
 * per glyph and stops at the first NUL or when dst_x + advance
 * would exceed fb_width. Returns the number of glyphs actually
 * painted. Non-ASCII bytes are skipped without advancing. */
unsigned int dm1_v1_fmtowns_font_rasterise_string_pc34(
    const uint8_t *raster,
    uint8_t       *fb,
    int            fb_width,
    int            fb_height,
    int            fb_stride,
    int            dst_x,
    int            dst_y,
    uint8_t        fg,
    uint8_t        bg,
    int            write_bg,
    const char    *ascii_string);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_FONT_RASTERISER_H */

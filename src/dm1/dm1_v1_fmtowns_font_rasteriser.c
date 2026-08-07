#include "dm1_v1_fmtowns_font_rasteriser.h"
#include "dm1_v1_fmtowns_text_geometry.h"

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
    uint8_t        ascii_byte) {
    int painted = 0;
    unsigned int row;
    if (!raster || !fb) return 0;
    if (fb_width <= 0 || fb_height <= 0 || fb_stride < fb_width) return 0;
    if (ascii_byte >= 128u) return 0;
    for (row = 0; row < (unsigned int)DM1_V1_FMTOWNS_CHAR_Y_SIZE; ++row) {
        int y = dst_y + (int)row;
        uint8_t bits;
        unsigned int col;
        if (y < 0 || y >= fb_height) continue;
        bits = raster[row * 128u + ascii_byte];
        for (col = 0; col < (unsigned int)DM1_V1_FMTOWNS_CHAR_X_SIZE; ++col) {
            int x = dst_x + (int)col;
            int set;
            if (x < 0 || x >= fb_width) continue;
            /* Glyph body is right-aligned in the 8-bit byte: the
             * leftmost visible pixel (col 0) is bit (CHAR_X_SIZE-1),
             * the rightmost (col 4) is bit 0. Verified by decoding
             * asset 557 from the shipped Japanese Track 01
             * GRAPHICS.DAT — 'A' row 3 has value 0x1f (bits 4..0
             * set) which paints columns 0..4 of a 5-wide crossbar. */
            set = (bits >> ((unsigned int)DM1_V1_FMTOWNS_CHAR_X_SIZE - 1u - col)) & 1u;
            if (set) {
                fb[y * fb_stride + x] = fg;
                painted = 1;
            } else if (write_bg) {
                fb[y * fb_stride + x] = bg;
                painted = 1;
            }
        }
    }
    return painted;
}

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
    const char    *ascii_string) {
    unsigned int painted = 0;
    int cursor;
    const char *p;
    if (!raster || !fb || !ascii_string) return 0;
    cursor = dst_x;
    for (p = ascii_string; *p != '\0'; ++p) {
        uint8_t b = (uint8_t)*p;
        if (b >= 128u) continue;
        if (cursor + DM1_V1_FMTOWNS_CHAR_X_SIZE > fb_width) break;
        if (dm1_v1_fmtowns_font_rasterise_glyph_pc34(
                raster, fb, fb_width, fb_height, fb_stride,
                cursor, dst_y, fg, bg, write_bg, b)) {
            ++painted;
        }
        cursor += DM1_V1_FMTOWNS_CHAR_X_WID;
    }
    return painted;
}

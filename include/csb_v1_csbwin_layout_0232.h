#ifndef FIRESTAFF_CSB_V1_CSBWIN_LAYOUT_0232_H
#define FIRESTAFF_CSB_V1_CSBWIN_LAYOUT_0232_H

#include <stddef.h>
#include <stdint.h>

/* CSBWin expands GRAPHICS.DAT item 0x232 to Data::Byte1830.  It is a
 * 0x722-byte configuration record, not a bitmap.  These offsets are relative
 * to Byte1830 and follow Data.h / CSBCode.cpp's post-expand byte swaps. */
#define CSB_V1_CSBWIN_LAYOUT_0232_DECODED_SIZE 0x722u

typedef struct {
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
} CSB_V1_CSBWinRect0232;

typedef struct {
    int valid;
    CSB_V1_CSBWinRect0232 party_direction[4];
    CSB_V1_CSBWinRect0232 eye_box;
    CSB_V1_CSBWinRect0232 mouth_box;
    CSB_V1_CSBWinRect0232 poison_box;
    CSB_V1_CSBWinRect0232 food_water_box;
    CSB_V1_CSBWinRect0232 movement_box;
    CSB_V1_CSBWinRect0232 magic_box;
} CSB_V1_CSBWinLayout0232;

/* Decode only the source-owned layout record.  The caller owns decompression
 * of graphic 0x232 and can decide independently whether its graphics package
 * is admitted for runtime use. */
int csb_v1_csbwin_layout_0232_decode(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_CSBWinLayout0232 *out_layout);

/* Read and LZW-decompress standard CSBWin/Atari GRAPHICS.DAT item 0x232,
 * then decode its source layout. The file must expose the original 563-item
 * DMCSB1 index and item 0x232 must expand to exactly 0x722 bytes. */
int csb_v1_csbwin_layout_0232_read_graphics_dat(
    const char *graphics_dat_path, CSB_V1_CSBWinLayout0232 *out_layout);

/* Rectangles use inclusive CSBWin screen coordinates. */
int csb_v1_csbwin_layout_0232_rect_is_screen_valid(
    const CSB_V1_CSBWinRect0232 *rect);

#endif

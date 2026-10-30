#ifndef CSB_V1_ATARI_F0133_INDEXED_BLIT_H
#define CSB_V1_ATARI_F0133_INDEXED_BLIT_H

#include <stdint.h>

/* ReDMCSB VIDEO.C F0133 tiles 8-byte source units, which are 16 indexed
 * pixels wide, across an inclusive destination box. `last_unit` is the
 * source restart point: after last_unit-1 the source wraps to unit zero. */
int csb_v1_atari_f0133_fill_indexed_box(
    const uint8_t *source_pixels,
    int source_width,
    int source_height,
    int last_unit,
    int first_unit,
    uint8_t *destination,
    int destination_stride,
    int destination_x,
    int destination_y,
    int box_width,
    int box_height,
    int transparent_color,
    const uint8_t palette_map[16]);

#endif

#ifndef FIRESTAFF_CSB_V1_CSBWIN_VIEWPORT_GRAPHICS_MAP_H
#define FIRESTAFF_CSB_V1_CSBWIN_VIEWPORT_GRAPHICS_MAP_H

#include <stdint.h>

/* CSBWin CSBCode.cpp:2933-2940: Atari/CSBWin uses 13 records per wall set,
 * starting at 77. Entries 0..6 are door bitmaps, 7..12 wall bitmaps. */
enum {
    CSB_V1_CSBWIN_FLOORSET_FIRST_GRAPHIC = 75,
    CSB_V1_CSBWIN_FLOORSET_GRAPHIC_COUNT = 2,
    CSB_V1_CSBWIN_WALLSET_FIRST_GRAPHIC = 77,
    CSB_V1_CSBWIN_WALLSET_GRAPHIC_COUNT = 13,
    CSB_V1_CSBWIN_DOOR_GRAPHIC_COUNT = 7,
    CSB_V1_CSBWIN_WALL_GRAPHIC_COUNT = 6
};

int csb_v1_csbwin_floor_ceiling_graphic_index(uint16_t floor_set,
                                               int ceiling,
                                               uint16_t *out_graphic_index);

/* CSBWin Bitmaps.cpp:187: two indexed 4-bit pixels occupy one source byte. */
int csb_v1_csbwin_packed_byte_width(uint16_t pixel_width,
                                    uint16_t *out_byte_width);

int csb_v1_csbwin_viewport_graphic_index(uint16_t wall_set,
                                          uint16_t slot,
                                          uint16_t *out_graphic_index);

#endif

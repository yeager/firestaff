#ifndef FIRESTAFF_CSB_V1_PC34_WALLSET_GRAPHICS_MAP_H
#define FIRESTAFF_CSB_V1_PC34_WALLSET_GRAPHICS_MAP_H

#include <stdint.h>

/*
 * ReDMCSB PC/I34 DUNVIEW.C:F0095_LoadWallSet.
 *
 * Each PC3.4 wall set occupies 40 consecutive GRAPHICS.DAT records. The
 * first 22 are the seven door-frame surfaces followed by G2107's fifteen
 * F0128 wall surfaces. The remaining 18 records are stairs and intentionally
 * have no entry in this wall-only map.
 */
#define CSB_V1_PC34_WALLSET_FIRST_GRAPHICS_ENTRY 86u
#define CSB_V1_PC34_WALLSET_GRAPHICS_ENTRY_COUNT 40u
#define CSB_V1_PC34_WALLSET_F0128_SURFACE_COUNT 22u

typedef enum {
    CSB_V1_PC34_WALLSET_DOOR_FRAME_FRONT_D0C = 0,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_LEFT_D1C,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_LEFT_D2C,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_LEFT_D3C,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_LEFT_D3L,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_TOP_D1LCR,
    CSB_V1_PC34_WALLSET_DOOR_FRAME_TOP_D2LCR,
    CSB_V1_PC34_WALLSET_WALL_D0R,
    CSB_V1_PC34_WALLSET_WALL_D0L,
    CSB_V1_PC34_WALLSET_WALL_D1R,
    CSB_V1_PC34_WALLSET_WALL_D1L,
    CSB_V1_PC34_WALLSET_WALL_D1C,
    CSB_V1_PC34_WALLSET_WALL_D2R2,
    CSB_V1_PC34_WALLSET_WALL_D2L2,
    CSB_V1_PC34_WALLSET_WALL_D2R,
    CSB_V1_PC34_WALLSET_WALL_D2L,
    CSB_V1_PC34_WALLSET_WALL_D2C,
    CSB_V1_PC34_WALLSET_WALL_D3R2,
    CSB_V1_PC34_WALLSET_WALL_D3L2,
    CSB_V1_PC34_WALLSET_WALL_D3R,
    CSB_V1_PC34_WALLSET_WALL_D3L,
    CSB_V1_PC34_WALLSET_WALL_D3C
} CSB_V1_PC34WallSetSurface;

/* Resolves an exact PC3.4 F0095/F0128 wall-set record. The caller supplies
 * the authenticated GRAPHICS.DAT catalog count, so an absent record fails
 * closed rather than borrowing a neighbouring wall set. */
int csb_v1_pc34_wallset_graphics_entry_index(
    int wall_set,
    CSB_V1_PC34WallSetSurface surface,
    uint32_t graphics_entry_count,
    uint32_t *out_entry_index);

#endif

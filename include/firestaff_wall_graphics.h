
#ifndef FIRESTAFF_WALL_GRAPHICS_H
#define FIRESTAFF_WALL_GRAPHICS_H

/* DM1 wall graphic indices in GRAPHICS.DAT.
 * Source: ReDMCSB DEFS.H wall set constants.
 * Each wall set has graphics for D0-D3 distances, L/C/R positions.
 *
 * Graphic layout in GRAPHICS.DAT (approximate — varies by wall set):
 *   #0:  viewport background (224x136)
 *   #1:  fullscreen background (320x200)
 *   #2-3: palette/UI data
 *   #4-5: title screens (320x200)
 *   #6-15: UI elements
 *   #16-29: panel graphics
 *   #30+: wall set bitmaps (D3L, D3C, D3R, D2L, D2C, D2R, D1L, D1C, D1R, D0L, D0C, D0R)
 *   #121+: wall ornaments
 *   #247+: floor ornaments
 *   #303+: door ornaments
 */

/* Default wall set start index */
#define FS_GFX_WALL_SET_START 30

/* Wall bitmap indices relative to wall set start */
#define FS_WALL_D3L  0
#define FS_WALL_D3C  1
#define FS_WALL_D3R  2
#define FS_WALL_D2L  3
#define FS_WALL_D2C  4
#define FS_WALL_D2R  5
#define FS_WALL_D1L  6
#define FS_WALL_D1C  7
#define FS_WALL_D1R  8
#define FS_WALL_D0L  9
#define FS_WALL_D0C  10
#define FS_WALL_D0R  11

/* Get graphic index for a wall at given distance and position */
static inline int fs_wall_graphic_index(int wall_set, int distance, int position) {
    /* distance: 0=D0(near), 1=D1, 2=D2, 3=D3(far)
     * position: 0=left, 1=center, 2=right */
    int base = FS_GFX_WALL_SET_START + wall_set * 12;
    int d_offset = (3 - distance) * 3; /* D3 first in set */
    return base + d_offset + position;
}

/* Special graphic indices */
#define FS_GFX_VIEWPORT_BG     0
#define FS_GFX_FULLSCREEN_BG   1
#define FS_GFX_TITLE_1         4
#define FS_GFX_TITLE_2         5
#define FS_GFX_VIEWPORT_FRAME  17

#endif

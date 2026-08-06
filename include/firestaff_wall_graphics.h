
#ifndef FIRESTAFF_WALL_GRAPHICS_H
#define FIRESTAFF_WALL_GRAPHICS_H

/* The old viewport bridge once guessed wall records with a fixed arithmetic
 * table.  That table is not a PC34 GRAPHICS.DAT contract: wall-set ownership
 * comes from the authenticated map/source presentation tables.  Keep this
 * compatibility helper as a no-draw sentinel until a source-owned caller is
 * bound; it must never manufacture an index that happens to decode. */
static inline int fs_wall_graphic_index(int wall_set, int distance, int position) {
    (void)wall_set;
    (void)distance;
    (void)position;
    return -1;
}

/* Special graphic indices */
#define FS_GFX_VIEWPORT_BG     0
#define FS_GFX_FULLSCREEN_BG   1
#define FS_GFX_TITLE_1         4
#define FS_GFX_TITLE_2         5
#define FS_GFX_VIEWPORT_FRAME  17

#endif

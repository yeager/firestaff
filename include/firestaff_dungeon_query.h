
#ifndef FIRESTAFF_DUNGEON_QUERY_H
#define FIRESTAFF_DUNGEON_QUERY_H
#include <stdint.h>

/* Query dungeon squares in the view cone for viewport rendering.
 * Given party position + direction, returns square types for
 * D0-D3 at left/center/right positions. */

#define FS_SQUARE_WALL       0
#define FS_SQUARE_CORRIDOR   1
#define FS_SQUARE_STAIRS_UP  2
#define FS_SQUARE_STAIRS_DN  3
#define FS_SQUARE_PIT        4
#define FS_SQUARE_TELEPORTER 5
#define FS_SQUARE_DOOR_FRONT 16
#define FS_SQUARE_DOOR_SIDE  17

typedef struct {
    int square_type[4][3]; /* [distance D0-D3][position L/C/R] */
    int has_wall[4][3];    /* 1 if wall bitmap should be drawn */
} FS_ViewCone;

/* Compute view cone from party position in dungeon grid.
 * dungeon = 32x32 array of square types, 0=wall, 1=corridor, etc. */
static inline void fs_dungeon_compute_view_cone(
    const uint8_t *dungeon, int map_w, int map_h,
    int px, int py, int dir, FS_ViewCone *cone)
{
    /* Direction vectors: N=0, E=1, S=2, W=3 */
    int fdx[] = {0, 1, 0, -1};  /* forward */
    int fdy[] = {-1, 0, 1, 0};
    int rdx[] = {1, 0, -1, 0};  /* right */
    int rdy[] = {0, 1, 0, -1};
    int d, p, qx, qy;

    if (!dungeon || !cone) return;

    for (d = 0; d < 4; d++) {
        for (p = 0; p < 3; p++) {
            /* p: 0=left(-1), 1=center(0), 2=right(+1) */
            int side = p - 1;
            qx = px + fdx[dir] * (d + 1) + rdx[dir] * side;
            qy = py + fdy[dir] * (d + 1) + rdy[dir] * side;

            if (qx < 0 || qx >= map_w || qy < 0 || qy >= map_h) {
                cone->square_type[d][p] = FS_SQUARE_WALL;
                cone->has_wall[d][p] = 1;
            } else {
                int sq = dungeon[qy * map_w + qx];
                cone->square_type[d][p] = sq;
                cone->has_wall[d][p] = (sq == FS_SQUARE_WALL);
            }
        }
    }
}


/* Load DUNGEON.DAT and parse level grids */
int fs_dungeon_load_dat(const uint8_t *data, int size);
void fs_dungeon_set_level(int level);
int fs_dungeon_get_square_type(int x, int y);
int fs_dungeon_get_door_type(int x, int y);
int fs_dungeon_get_door_state(int x, int y);
int fs_dungeon_get_wall_ornament(int x, int y, int dir);
int fs_dungeon_get_floor_ornament(int x, int y);
int fs_dungeon_get_width(void);
int fs_dungeon_get_height(void);
const uint8_t *fs_dungeon_get_grid(void);
int fs_dungeon_get_start_x(void);
int fs_dungeon_get_start_y(void);
int fs_dungeon_get_start_dir(void);

#endif

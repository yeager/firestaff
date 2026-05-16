
#include "firestaff_dungeon_query.h"
#include "firestaff_wall_graphics.h"

/* Forward declarations for functions defined elsewhere */
int fs_dungeon_get_door_type(int x, int y);
int fs_dungeon_get_door_state(int x, int y);
int fs_dungeon_get_wall_ornament(int x, int y, int dir);
int fs_dungeon_get_floor_ornament(int x, int y);

#include "firestaff_graphics_dat_reader.h"
#include <string.h>
#include <stdio.h>

/* Bridge: DUNGEON.DAT square types → view cone → wall bitmap selection.
 *
 * Square types from DM1 DUNGEON.DAT:
 *   0 = wall (solid), 1 = open floor, 2 = pit,
 *   3 = stairs, 4 = door, 5 = teleporter
 *
 * For each square in view cone (D0-D3, left/center/right):
 *   1. Read square type from dungeon data
 *   2. Determine wall presence on each face
 *   3. Select wall bitmap index from GRAPHICS.DAT
 *   4. Queue for V1 viewport renderer */

#define VP_VIEW_DEPTH 4
#define VP_VIEW_WIDTH 3

typedef struct {
    int square_type;
    int wall_north, wall_east, wall_south, wall_west;
    int ornament_index;      /* wall ornament (torch, switch, etc) */
    int floor_ornament;
    int ceil_ornament;
    int door_type;           /* 0=none, 1=wood, 2=iron, 3=Ra */
    int door_state;          /* 0=closed, 1=open, 2=destroyed */
} VP_SquareInfo;

/* Direction offsets: N=0, E=1, S=2, W=3 */
static const int g_dx[4] = {0, 1, 0, -1};
static const int g_dy[4] = {-1, 0, 1, 0};

/* Build view cone square info */
int fs_viewport_build_view_cone(
    int party_x, int party_y, int party_dir,
    VP_SquareInfo view_cone[VP_VIEW_DEPTH][VP_VIEW_WIDTH])
{
    int depth, col;
    int left_dir = (party_dir + 3) & 3;
    int count = 0;

    for (depth = 0; depth < VP_VIEW_DEPTH; depth++) {
        for (col = -1; col <= 1; col++) {
            int x = party_x + g_dx[party_dir] * depth + g_dx[left_dir] * col;
            int y = party_y + g_dy[party_dir] * depth + g_dy[left_dir] * col;
            VP_SquareInfo *sq = &view_cone[depth][col + 1];
            memset(sq, 0, sizeof(*sq));

            sq->square_type = fs_dungeon_get_square_type(x, y);

            /* Walls: present if neighbor is solid or at map boundary */
            sq->wall_north = (fs_dungeon_get_square_type(x, y-1) == 0) ? 1 : 0;
            sq->wall_south = (fs_dungeon_get_square_type(x, y+1) == 0) ? 1 : 0;
            sq->wall_east  = (fs_dungeon_get_square_type(x+1, y) == 0) ? 1 : 0;
            sq->wall_west  = (fs_dungeon_get_square_type(x-1, y) == 0) ? 1 : 0;

            /* Door info */
            if (sq->square_type == 4) {
                sq->door_type = fs_dungeon_get_door_type(x, y);
                sq->door_state = fs_dungeon_get_door_state(x, y);
            }

            /* Ornaments */
            sq->ornament_index = fs_dungeon_get_wall_ornament(x, y, party_dir);
            sq->floor_ornament = fs_dungeon_get_floor_ornament(x, y);

            count++;
        }
    }
    return count;
}

/* Wall bitmap selection from GRAPHICS.DAT index */
int fs_viewport_select_wall_bitmap(int wall_type, int distance, int position) {
    /* DM1 PC34 GRAPHICS.DAT wall bitmap indices:
     * Base wall textures start at index 300+
     * Distance 0 (closest) uses largest variants
     * Distance 3 (farthest) uses smallest
     * Position: 0=left, 1=center, 2=right */
    int base = 300;
    int dist_offset = distance * 18;  /* 18 variants per distance */
    int pos_offset = position * 6;
    return base + dist_offset + pos_offset + wall_type;
}


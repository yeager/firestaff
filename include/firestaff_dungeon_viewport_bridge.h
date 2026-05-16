
#ifndef FIRESTAFF_DUNGEON_VIEWPORT_BRIDGE_H
#define FIRESTAFF_DUNGEON_VIEWPORT_BRIDGE_H

#define VP_VIEW_DEPTH 4
#define VP_VIEW_WIDTH 3

typedef struct {
    int square_type;
    int wall_north, wall_east, wall_south, wall_west;
    int ornament_index;
    int floor_ornament;
    int ceil_ornament;
    int door_type;
    int door_state;
} VP_SquareInfo;

int fs_viewport_build_view_cone(int party_x, int party_y, int party_dir,
    VP_SquareInfo view_cone[VP_VIEW_DEPTH][VP_VIEW_WIDTH]);
int fs_viewport_select_wall_bitmap(int wall_type, int distance, int position);

#endif


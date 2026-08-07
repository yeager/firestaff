#include "dm1_v2_pathfinding_pc34.h"

void v2_path_init(void) {
}

void v2_path_clear(void) {
}

bool v2_path_find(int* map, int map_w, int map_h, int sx, int sy, int gx, int gy, M11_V2_Path* out) {
    (void)map;
    (void)map_w;
    (void)map_h;
    (void)sx;
    (void)sy;
    (void)gx;
    (void)gy;
    (void)out;

    /*
     * PC34 resolves each creature move through GROUP.C F0202/F0203 against
     * dungeon state and creature attributes.  It has no general grid-path
     * service, so do not fabricate a route from an unowned integer map.
     */
    return false;
}

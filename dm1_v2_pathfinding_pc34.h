#ifndef FIRESTAFF_DM1_V2_PATHFINDING_PC34_H
#define FIRESTAFF_DM1_V2_PATHFINDING_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct M11_V2_PathNode {
    int x, y;
    int g, h, f;
    int parent_idx;
    bool open, closed;
} M11_V2_PathNode;

typedef struct M11_V2_Path {
    int steps[128][2];
    int length;
    bool valid;
} M11_V2_Path;

void v2_path_init(void);
bool v2_path_find(int* map, int map_w, int map_h, int sx, int sy, int gx, int gy, M11_V2_Path* out);
void v2_path_clear(void);

#endif

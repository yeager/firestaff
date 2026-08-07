#include "dm1_v2_pathfinding_pc34.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    int map[16] = {0};
    M11_V2_Path path;
    M11_V2_Path before;

    memset(&path, 0xA5, sizeof(path));
    memcpy(&before, &path, sizeof(path));
    v2_path_init();
    if (v2_path_find(map, 4, 4, 0, 0, 3, 3, &path)) return 1;
    v2_path_clear();
    if (memcmp(&path, &before, sizeof(path)) != 0) return 1;
    puts("dm1_v2_pathfinding_pc34: ok");
    return 0;
}


#include "nexus_v1_fountain.h"
#include <string.h>

void nexus_v1_fountain_manager_init(Nexus_FountainManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_fountain_register(Nexus_FountainManager *mgr,
    int type, int map_x, int map_y,
    int uses, int restore_amount) {
    /* No retail Nexus fountain record, effect value, or Saturn action
     * dispatcher has been authenticated. The old helper accepted caller
     * supplied DM1-shaped values and made them live world objects. Retain
     * the manager as a capture/import seam, but never publish a guessed
     * fountain into production state. */
    (void)mgr;
    (void)type;
    (void)map_x;
    (void)map_y;
    (void)uses;
    (void)restore_amount;
    return -1;
}

int nexus_v1_fountain_find_at(const Nexus_FountainManager *mgr,
    int map_x, int map_y) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->fountains[i].active &&
            mgr->fountains[i].map_x == map_x &&
            mgr->fountains[i].map_y == map_y)
            return i;
    }
    return -1;
}

int nexus_v1_fountain_drink(Nexus_FountainManager *mgr,
    int fountain_idx, Nexus_V1_Champion *champion) {
    /* ITEM/DGN bytes do not prove a fountain action or effect magnitude. */
    (void)mgr;
    (void)fountain_idx;
    (void)champion;
    return 0;
}

int nexus_v1_fountain_uses_left(const Nexus_FountainManager *mgr,
    int fountain_idx) {
    if (!mgr || fountain_idx < 0 || fountain_idx >= mgr->count) return 0;
    return mgr->fountains[fountain_idx].uses_remaining;
}
